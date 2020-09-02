#include "link_settings.h"
#include "ui_link_settings.h"
#include <QDebug>
#include <QTime>
#include "mainwindow.h"
#include <array>

#define IDLE 0
#define READING 1

link_settings::link_settings(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::link_settings)
{
    ui->setupUi(this);
    agent = new QBluetoothDeviceDiscoveryAgent;
    connect(agent,SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
             this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)) );

    agent->start(); //It seems like it is getting us into trouble

    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    connect(socket, SIGNAL(connected()), this, SLOT(slot_connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
    connect(socket, SIGNAL(readyRead()),this,SLOT(slot_read()));
    connect(ui->buttonOBD, SIGNAL(toggled(bool)), this, SLOT(slot_radioKWPOBD()));
    connect(ui->buttonRHY, SIGNAL(toggled(bool)), this, SLOT(slot_radioKWPRHY()));
    connect(ui->buttonCAN, SIGNAL(toggled(bool)), this, SLOT(slot_radioCAN()));
    connect(ui->readButton, SIGNAL(clicked()), this, SLOT(slot_readButton()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(slot_clearButton()));

    serviceUUid = "00001101-0000-1000-8000-00805F9B34FB";
    ui->listWidget_2->setEnabled(false);

    mutex = new QMutex();
    mutex->lock();

    dc = new dataContainer();

    workerThread = new QThread;
    worker = new Worker(dc,mutex,socket);
    worker->moveToThread(workerThread);

    connect(worker,SIGNAL(newData()),this,SLOT(processData()));
    connect(workerThread,SIGNAL(started()),worker,SLOT(process()));
    workerThread->start();
    workerThread->setPriority(QThread::HighPriority);
}

link_settings::~link_settings()
{
    delete dc;
    delete worker;
    delete ui;
}

void link_settings::end_comms()
{
    dataToSend = "STOP";
    socket->write(dataToSend);
    socket->abort();
    socket->close();
    agent->stop();
}

void link_settings::slot_connected()
{
    socket->write(dataToSend);
}

void link_settings::slot_disconnected()
{
    qDebug() << "Device disconnected" << endl;
    socket->close();
    agent->stop();
    exit(1);
}

void link_settings::slot_error(QBluetoothSocket::SocketError error)
{
    qDebug() << error << endl;
}

void link_settings::slot_read()
{
    // First step: we copy data from the buffer to the container class.
    mutex->unlock();
}

void link_settings::processData()
{
    QByteArray data;
    dc->getData(data);
    bool cs = checkCs(data);

    switch (c_mode) {
        case KWPOBD:
            process_KWPOBDData(data);
        break;
    case KWPSiemens:
            process_KWPRHYData(data);
        break;
    case CANOBD:
        // Reserved for future expansion.
        break;
    }
}

bool link_settings::checkCs(const QByteArray &dataIn)
{
    uint16_t cs = 0;
    uint8_t l = (dataIn.at(0) & 0x3F) + 4;
    for(uint8_t i = 0; i < (l-1); i++)
    {
        cs += dataIn.at(i);
    }
    return ((uint8_t)(cs&0x00FF) == (uint8_t)dataIn.at(l-1));
}

void link_settings::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    ui->listWidget->addItem(device.name());
    ui->listWidget_2->addItem(device.address().toString());
}

void link_settings::on_listWidget_itemClicked(QListWidgetItem *item)
{
    name = item->text();
    int pos = ui->listWidget->currentRow();
    addr = ui->listWidget_2->item(pos)->text();
    connectBT(addr, getMode());
}

void link_settings::connectBT(const QString &addr, connection mode)
{
    socket->connectToService(QBluetoothAddress(addr),QBluetoothUuid(serviceUUid),QIODevice::ReadWrite);
    if(mode == connection::CANOBD)
        slot_radioCAN();
    else if(mode == connection::KWPOBD)
        slot_radioKWPOBD();
    else if(mode == connection::KWPSiemens)
        slot_radioKWPRHY();
}

void link_settings::slot_radioKWPRHY()
{
    dataToSend = "RHY";
    ui->readButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    socket->write(dataToSend);
}

void link_settings::slot_radioCAN()
{
    dataToSend = "CAN";
    ui->readButton->setEnabled(true);
    ui->clearButton->setEnabled(true);
    socket->write(dataToSend);
}

void link_settings::slot_radioKWPOBD()
{
    dataToSend = "OBD";
    ui->readButton->setEnabled(true);
    ui->clearButton->setEnabled(true);
    socket->write(dataToSend);
}

void link_settings::slot_clearButton()
{
    dataToSend = "CLEAR";
    socket->write(dataToSend);
}

void link_settings::slot_readButton()
{
    dataToSend = "READ";
    socket->write(dataToSend);
}

link_settings::connection link_settings::getMode()
{
    if(ui->buttonCAN->isChecked())
        return CANOBD;
    else if(ui->buttonOBD->isChecked())
        return KWPOBD;
    else if(ui->buttonRHY->isChecked())
        return KWPSiemens;
}

QString link_settings::getLastAddr()
{
    return addr;
}

void link_settings::setAddr(QString address)
{
    addr = address;
}

void link_settings::setMode(int mode)
{
    c_mode = mode;
}

int dataContainer::find_header(QByteArray &data, int len, int pos)
{
    bool found = false;
    uint8_t curr, next,prev;
    prev = 0;
    uint nextpos;
    while(!found)
    {
        nextpos = (pos+1) % len;
        curr = (uint8_t)data.at(pos);
        next = (uint8_t)data.at(nextpos);

        if( (curr >> 7) & 0x01)
        {
            if( (next == 0xF1) || (prev == 0x0A) )
                found = true;
            else if(pos == (len-1))
                found = true;
        }
        else
        {
            if(pos < (len - 1))
                pos++;
            else
                return -1;
        }
        prev = curr;
    }
        return pos;
}

bool Worker::find_rpr_end(const QByteArray &data, int &ptr, int &len)
{
    int p1 = data.indexOf(0x41,0);
    int p2 = data.indexOf(0x0A,0);
    int p3 = p2-1;
    if( (data.size() < 4) || (p2 < 0) )
        return false;
    if((data.at(p3)==0x0D) && (p1 >= 0) )
    {
        ptr = p1;
        len = p3 - p1 + 1;
        return true;
    }
    return false;
}

uint8_t dataContainer::decode_length(uint8_t val)
{
    return ( (val & 0x3F) + 6 ) ;
}

int dataContainer::getRawSize()
{
    return raw_data.size();
}

int dataContainer::getDataSize()
{
    return process_data.size();
}

void link_settings::process_KWPOBDData(const QByteArray &data)
{
    //Live data:
    if((int)data[3] == 0x41)
    {
        switch(data[4])
        {
            case WATER_TEMP:
            {
                    int v = data[5] - 40;
                    emit water_tempChanged(v);
                break;
            }
            case FUEL_PRESS:
            {
                    int v = 10*(256*data[5]+data[6])/1000;
                    emit fuel_pressChanged(v);
                break;
            }
            case INTAKE_PRESS:
            {
                    unsigned int v = (unsigned int) data[5];
                    emit turbo_pressChanged(v);
                break;
            }
            case ENGINE_RPM:
            {
                    double v = (double)( 256 * data[5] + data[6])/4;
                    v = v / 1000;
                    emit RPM_valueChanged(v);
                break;
            }
            case SPEED:
            {
                   uint8_t v = (uint8_t)data[5];
                   emit speedChanged(v);
                break;
            }
            case MAF_RATE:
            {
                    double v = (double) (256 * data[5] + data[6])/100;
                    emit maf_rateChanged(v);
                break;
            }

            //Oil thermometer is not implemented yet

        }
    }
}

void link_settings::process_KWPRHYData(const QByteArray &data)
{
    if( ((uint8_t)data[4]) == 0xC4)
    {
        int v = data[12] - 40; // Coolant value.
        emit water_tempChanged(v);
    }
}

dataContainer::dataContainer()
{
    raw_mutex = new QMutex();
    process_mutex = new QMutex();
    raw_data.resize(0);
    process_data.resize(0);
}

dataContainer::~dataContainer()
{

}

void dataContainer::getData(QByteArray &data)
{
    process_mutex->lock();
    data = process_data;
    process_mutex->unlock();
}

void dataContainer::getRawData(QByteArray &data)
{
    raw_mutex->lock();
    data = raw_data;
    raw_mutex->unlock();
}

void dataContainer::setData(const QByteArray &data)
{
    process_mutex->lock();
    process_data.resize(data.size());
    process_data = data;
    process_mutex->unlock();
}

void dataContainer::setRawData(const QByteArray &data)
{
    raw_mutex->lock();
    raw_data.resize(data.size());
    raw_data = data;
    raw_mutex->unlock();
}

Worker::Worker(dataContainer *dataC, QMutex *mu, QBluetoothSocket *bs)
{
    dc = dataC;
    mutex = mu;
    socket = bs;
}

Worker::~Worker()
{

}

void Worker::process()
{
    int capable = 0;
    state = IDLE;
    QByteArray dataIn;
    while(1)
    {
        mutex->lock();
        dataIn.append(socket->readAll());
        if(dataIn.isEmpty())
            continue;
        if(state == IDLE)
        {
            if( (headerPos = dc->find_header(dataIn,dataIn.size(),0)) >= 0)
            {
                len = dc->decode_length(dataIn.at(headerPos));
                leftToRead = len;
                state = READING;
            }
            else
            {
                int first;
                int l = 0;
                if(find_rpr_end(dataIn,first,l))
                {
                    QByteArray data;
                    data.resize(l+3);
                    data.insert(3,dataIn.mid(first,l));
                    dc->setData(data);
                    emit newData();
                }

            }
        }
        if(state == READING)
        {
            capable = dataIn.size() - headerPos;
            if(leftToRead == len)
            {
                ///In this case we got a new frame.
                if(capable == len)
                {
                    //If we are able to get an entire frame.
                    bytesRead = len;
                    dc->setData(dataIn.mid(headerPos,bytesRead));
                    state = IDLE;
                    if(dc->getDataSize() < len)
                        while(1);
                    emit newData();//emits signal

                }
                else if(capable < len)
                {
                    //If we are not able to get an entire frame.
                    tempStore.resize(len);
                    bytesRead = capable;
                    tempStore = dataIn.mid(headerPos,bytesRead);
                    leftToRead -= capable;
                    state = READING;
                }
                else if(capable > len)
                {
                    //If we got more than was necessary, so we got more than a frame.
                    bytesRead = leftToRead;
                    dc->setData(dataIn.mid(headerPos,bytesRead));
                    state = IDLE;
                    if(dc->getDataSize() < len)
                        while(1);
                    emit newData();//emit signal.
                    mutex->unlock();

                }

            }
            else
            {
                ///Leftover frame.
                capable = dataIn.size();
                if(capable == leftToRead)
                {
                    bytesRead = leftToRead;
                    tempStore.append(dataIn.mid(0,bytesRead));
                    dc->setData(tempStore);
                    state = IDLE;
                    if(dc->getDataSize() < len)
                        while(1);
                    emit newData();//emit signal;

                }
                else if(capable < leftToRead)
                {
                    bytesRead = capable;
                    tempStore = dataIn.mid(len-leftToRead,bytesRead);
                    leftToRead -= capable;
                    state = READING;
                }
                else if(capable > leftToRead)
                {
                    bytesRead = leftToRead;
                    tempStore.append(dataIn.mid(0,bytesRead));
                    //data = tempStore;
                    dc->setData(tempStore);
                    state = IDLE;
                    if(dc->getDataSize() < len)
                        while(1); //it gets stuck here.
                    emit newData();//emits signal.
                    mutex->unlock();

                }

            }
            dataIn.remove(0,bytesRead+headerPos);
        }
    }
}

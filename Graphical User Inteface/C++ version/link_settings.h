#ifndef LINK_SETTINGS_H
#define LINK_SETTINGS_H

#include <QMainWindow>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QListWidgetItem>
#include <QThread>
#include <QMutexLocker>

#define WATER_TEMP 0x05
#define ENGINE_RPM 0x0C
#define FUEL_PRESS 0x23
#define INTAKE_PRESS 0x0B
#define SPEED 0x0D
#define MAF_RATE 0x10
#define MAX_DATA_LENGTH 64

namespace Ui {
class link_settings;
class Worker;
class dataContainer;
}

class dataContainer: public QObject
{
    Q_OBJECT
public:
    dataContainer();
    ~dataContainer();
    void getData(QByteArray &data);
    void getRawData(QByteArray &data);
    int  getDataSize();
    int  getRawSize();
    void setData(const QByteArray &data);
    void setRawData(const QByteArray &data);
    static int find_header(QByteArray &data, int len, int pos);
    static uint8_t decode_length(uint8_t  val);

signals:
    void updatedData();

private:
    QByteArray process_data;
    QByteArray raw_data;
    QMutex *raw_mutex, *process_mutex;
};

class Worker: public QObject{
    Q_OBJECT
public:
    Worker(dataContainer *dataC, QMutex *mu, QBluetoothSocket *bs);
    static bool find_rpr_end(const QByteArray &data, int &ptr, int &len);
    ~Worker();

public slots:
    void process();

signals:
        void newData();
private:
        unsigned int state;
        QByteArray tempStore;
        int leftToRead,len;
        int headerPos;
        int bytesRead;
        dataContainer *dc;
        QMutex *mutex;
        QBluetoothSocket *socket;
};

class link_settings : public QMainWindow
{
    Q_OBJECT

public:
    enum connection{
        KWPOBD=1,
        KWPSiemens,
        CANOBD
    };
    explicit link_settings(QWidget *parent = nullptr);
    ~link_settings();
    void connectBT(const QString &addr, connection mode);
    void end_comms();
    connection getMode();
    QString getLastAddr();
    void setAddr(QString address);
    void setMode(int mode);
    void RXThread();
    QMutex *mutex;

private slots:
    void slot_connected();
    void slot_disconnected();
    void slot_error(QBluetoothSocket::SocketError error);
    void process_KWPOBDData(const QByteArray &data);
    void process_KWPRHYData(const QByteArray &data);
    void slot_radioKWPRHY();
    void slot_radioCAN();
    void slot_radioKWPOBD();
    void slot_clearButton();
    void slot_readButton();
    void slot_read();
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void on_listWidget_itemClicked(QListWidgetItem *item);


signals:
    void speedChanged(int value);
    void fuel_pressChanged(int value);
    void turbo_pressChanged(double value);
    void maf_rateChanged(double value);
    void water_tempChanged(int value);
    void oil_tempChanged(int value);
    void RPM_valueChanged(double value);

public slots:
    void processData();

private:
    unsigned int newMessage_handler(QByteArray &data, unsigned int starting, unsigned int &mesLen);
    void copyLeft(QByteArray &dataIn, unsigned int len, unsigned int starting);
    bool checkCs(const QByteArray &dataIn);

    QThread *workerThread;
    Worker *worker;
    dataContainer *dc;
    Ui::link_settings *ui;
    QBluetoothDeviceDiscoveryAgent *agent;
    QBluetoothSocket *socket;
    QString addr,name;
    QString serviceUUid;
    QByteArray dataToSend;
    int c_mode;
};





#endif // LINK_SETTINGS_H

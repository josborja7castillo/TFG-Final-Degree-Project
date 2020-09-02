#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QDateTime>
#include <QPixmap>
#include "link_settings.h"
#include "menu_widgets.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{


    timer = new QTimer(this);
    ui->setupUi(this);
    updateTime();

    ls = new link_settings();

    readSettings();

    fill_menus();

    connect(timer,SIGNAL(timeout()),this,SLOT(updateTime()));
    connect(ui->actionExit,SIGNAL(triggered()), this, SLOT(end_gui()));
    connect(ui->actionLink_Configuration,SIGNAL(triggered()),this,SLOT(show_link_config()));

    connect(ui->actionTachometer, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ui->actionSpeedometer, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ui->actionDate_and_time, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ui->actionPressure_gauge, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ui->actionOil_temperature, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ui->actionWater_temperature, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ui->actionTurbo_pressure, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ui->actionMAF_rate, SIGNAL(triggered()), this, SLOT(visible_items()));
    connect(ls,SIGNAL(speedChanged(int)),this,SLOT(update_speed(int)));
    connect(ls,SIGNAL(fuel_pressChanged(int)),this,SLOT(update_fuel_press(int)));
    connect(ls,SIGNAL(turbo_pressChanged(unsigned int)),this,SLOT(update_turbo_press(unsigned int)));
    connect(ls,SIGNAL(maf_rateChanged(double)),this,SLOT(update_maf_rate(double)));
    connect(ls,SIGNAL(water_tempChanged(int)),this,SLOT(update_water_temp(int)));
    connect(ls,SIGNAL(RPM_valueChanged(double)),this,SLOT(update_RPM_value(double)));

    timer->start(1000);
    ui->tachometer->setDigitFormat("%g"); //It prevents showing number.00
    ui->speedLCD->setStyleSheet("background-color: transparent;");
    ui->fuel_label->setStyleSheet("background-color: transparent;");
    ui->turbo_label->setStyleSheet("background-color: transparent;");
    ls->connectBT(ls->getLastAddr(),ls->getMode());

    ui->speedLCD->display(0);
    ui->water_thermometer->setValue(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTime()
{
    QDate date = QDate::currentDate();
    ui->day_label->setText(date.toString("dddd d MMMM yyyy"));
    QTime time = QTime::currentTime();
    ui->time_label->setText(time.toString("hh:mm"));
}

void MainWindow::update_speed(int value)
{
    ui->speedLCD->display(value);
}

void MainWindow::update_fuel_press(int value)
{
    ui->fuel_gauge->setValue(value);
}
 
void MainWindow::update_turbo_press(unsigned int val)
{
    ui->turbo_manometer->setValue((double)val * 0.01);
}

void MainWindow::update_maf_rate(double val)
{
    ui->MAF->setValue(val);
}

void MainWindow::update_water_temp(int val)
{
    ui->water_thermometer->setValue(val);
}

void MainWindow::update_oil_temp(int val)
{
    ui->oil_thermometer->setValue(val);
}
 
void MainWindow::update_RPM_value(qreal value)
{
    ui->tachometer->setValue(value);
}

void MainWindow::set_tacho_limit(int value)
{
    ui->tachometer->setMajorTicks(value+1);
    ui->tachometer->setMinorTicks(5);
    ui->tachometer->setRange(0,value);
}

void MainWindow::set_oil_alarm(int value)
{
    ui->oil_thermometer->setCritical( (double) value);
}

void MainWindow::set_water_alarm(int value)
{
    ui->water_thermometer->setCritical( (double) value);
}

void MainWindow::fill_menus()
{
    ui->actionTachometer->setChecked(true);
    ui->actionSpeedometer->setChecked(true);
    ui->actionDate_and_time->setChecked(true);
    ui->actionPressure_gauge->setChecked(true);
    ui->actionTurbo_pressure->setChecked(true);
    ui->actionOil_temperature->setChecked(false);
    ui->actionWater_temperature->setChecked(true);
    ui->actionMAF_rate->setChecked(true);

    menu_tacho = new menu_spinbox("Set tachometer limit");
    menu_tacho->ret_spinBox()->setRange(3,12);
    menu_tacho->ret_spinBox()->setValue((int) ui->tachometer->getMaxValue());
    connect(menu_tacho->ret_spinBox(), SIGNAL(valueChanged(int)), this, SLOT(set_tacho_limit(int)));
    ui->menuSet_ranges->addAction(menu_tacho);

    menu_oil = new menu_spinbox("Set oil temperature critical point");
    menu_oil->ret_spinBox()->setRange(ui->oil_thermometer->minimum(),ui->oil_thermometer->maximum());
    menu_oil->ret_spinBox()->setValue(110);
    connect(menu_oil->ret_spinBox(), SIGNAL(valueChanged(int)), this, SLOT(set_oil_alarm(int)));
    ui->menuSet_ranges->addAction(menu_oil);

    menu_water = new menu_spinbox("Set water temperature critical point");
    menu_water->ret_spinBox()->setRange(ui->water_thermometer->minimum(),ui->water_thermometer->maximum());
    menu_water->ret_spinBox()->setValue(110);
    connect(menu_water->ret_spinBox(), SIGNAL(valueChanged(int)), this, SLOT(set_water_alarm(int)));
    ui->menuSet_ranges->addAction(menu_water);

}

void MainWindow::visible_items()
{
    ui->tachometer->setVisible(ui->actionTachometer->isChecked());
    ui->speedLCD->setVisible(ui->actionSpeedometer->isChecked());
    ui->kmph_label->setVisible(ui->actionSpeedometer->isChecked());

    ui->fuel_label->setVisible(ui->actionPressure_gauge->isChecked());
    ui->fuel_gauge->setVisible(ui->actionPressure_gauge->isChecked());

    ui->oil_label->setVisible(ui->actionOil_temperature->isChecked());
    ui->oil_thermometer->setVisible(ui->actionOil_temperature->isChecked());

    ui->water_label->setVisible(ui->actionWater_temperature->isChecked());
    ui->water_thermometer->setVisible(ui->actionWater_temperature->isChecked());

    ui->turbo_label->setVisible(ui->actionTurbo_pressure->isChecked());
    ui->turbo_manometer->setVisible(ui->actionTurbo_pressure->isChecked());

    ui->day_label->setVisible(ui->actionDate_and_time->isChecked());
    ui->time_label->setVisible(ui->actionDate_and_time->isChecked());

    ui->MAF->setVisible(ui->actionMAF_rate->isChecked());
    ui->maf_label->setVisible(ui->actionMAF_rate->isChecked());

    ui->stop_label->setVisible(false); //Blink if alarm
}

void MainWindow::readSettings()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString filename = "config.ini";

    QSettings *settings = new QSettings(path+"/"+filename,QSettings::NativeFormat);

    settings->beginGroup("mainwindow");
    set_tacho_limit(settings->value("tachometerVal",7).toInt());
    ui->tachometer->setVisible( settings->value("tachoVisible",true).toBool() );
    ui->speedLCD->setVisible( settings->value("speedoVisible",true).toBool() );
    ui->kmph_label->setVisible( settings->value("speedLabelVisible",true).toBool() );
    ui->time_label->setVisible( settings->value("timeVisible",false).toBool() );
    ui->day_label->setVisible( settings->value("dateVisible",false).toBool() );
    ui->water_thermometer->setVisible( settings->value("waterTempVisible",true).toBool() );
    ui->water_label->setVisible( settings->value("waterLabelVisible",true).toBool() );
    ui->oil_thermometer->setVisible( settings->value("oilTempVisible",false).toBool() );
    ui->oil_label->setVisible( settings->value("oilLabelVisible",false).toBool() );
    ui->fuel_gauge->setVisible( settings->value("fuelVisible",true).toBool() );
    ui->fuel_label->setVisible( settings->value("fuelLabelVisible",true).toBool() );
    ui->turbo_manometer->setVisible( settings->value("turboVisible",false).toBool() );
    ui->turbo_label->setVisible( settings->value("turboLabelVisible",false).toBool() );
    ui->MAF->setVisible( settings->value("mafVisible",true).toBool() );
    ui->maf_label->setVisible( settings->value("mafLabelVisible",true).toBool() );
    ui->stop_label->setVisible( settings->value("stopVisible",false).toBool() );
    ui->stop_label->setStyleSheet( settings->value("stopStyle","background-color: transparent; color: red;").toString() );
    ls->setAddr(settings->value("peripheral").toString());
    ls->setMode(settings->value("mode").toInt());
    settings->endGroup();
}

void MainWindow::writeSettings()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString filename = "config.ini";

    QSettings *settings = new QSettings(path+"/"+filename,QSettings::NativeFormat);
    settings->beginGroup("mainwindow");
    settings->setValue("size",window()->size());
    settings->setValue("tachometerVal",(int) ui->tachometer->getMaxValue() );
    settings->setValue("tachoVisible",ui->tachometer->isVisible());
    settings->setValue("speedoVisible",ui->speedLCD->isVisible());
    settings->setValue("speedoLabelVisible",ui->kmph_label->isVisible());
    settings->setValue("timeVisible",ui->time_label->isVisible());
    settings->setValue("dateVisible",ui->day_label->isVisible());
    settings->setValue("waterTempVisible",ui->water_thermometer->isVisible());
    settings->setValue("waterLabelVisible",ui->water_label->isVisible());
    settings->setValue("oilTempVisible",ui->oil_thermometer->isVisible());
    settings->setValue("oilLabelVisible",ui->oil_label->isVisible());
    settings->setValue("fuelVisible",ui->fuel_gauge->isVisible());
    settings->setValue("fuelLabelVisible",ui->fuel_label->isVisible());
    settings->setValue("turboVisible",ui->turbo_manometer->isVisible());
    settings->setValue("turboLabelVisible",ui->turbo_label->isVisible());
    settings->setValue("mafVisible",ui->MAF->isVisible());
    settings->setValue("mafLabelVisible",ui->maf_label->isVisible());
    settings->setValue("stopVisible",ui->stop_label->isVisible());
    settings->setValue("stopStyle","background-color: transparent; color: red;");
    settings->setValue("peripheral",ls->getLastAddr());
    settings->setValue("mode",ls->getMode());
    settings->sync();
    settings->endGroup();
}

void MainWindow::show_link_config()
{
    ls->show();
}

void MainWindow::end_gui()
{
    writeSettings();
    ls->end_comms(); //Ends bluetooth connection
    exit(0);
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "link_settings.h"
#include <QMainWindow>
#include <QTimer>
#include <QAction>
#include <QSpinBox>
#include <QLayout>
#include <QSettings>
#include <QStandardPaths>
#include "menu_widgets.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void writeSettings();
    void readSettings();



public slots:
    void updateTime();
    void set_tacho_limit(int value);
    void set_oil_alarm(int value);
    void set_water_alarm(int value);

    void update_speed(int value);
    void update_fuel_press(int value);
    void update_turbo_press(unsigned int val);
    void update_maf_rate(double val);
    void update_water_temp(int val);
    void update_oil_temp(int val);
    void update_RPM_value(double value);

private slots:
    void end_gui();
    void show_link_config();
    void visible_items();

private:
    void fill_menus();

    Ui::MainWindow *ui;
    QTimer *timer;
    menu_spinbox *menu_tacho, *menu_water, *menu_oil;
    link_settings *ls;
};
#endif // MAINWINDOW_H

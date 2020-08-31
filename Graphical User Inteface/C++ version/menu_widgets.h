#ifndef MENU_WIDGETS_H
#define MENU_WIDGETS_H

#include <QObject>
#include <QWidget>
#include <QSpinBox>
#include <QWidgetAction>
#include <QString>

class menu_spinbox: public QWidgetAction
{
public:
    menu_spinbox(const QString &title);
    QSpinBox *ret_spinBox();

private:
    QSpinBox *SpinBox;
};

class double_menu_spinbox: public QWidgetAction
{
public:
    double_menu_spinbox(const QString &title);
    QDoubleSpinBox *ret_spinBox();

private:
    QDoubleSpinBox *SpinBox;
};

#endif // MENU_WIDGETS_H

#include "menu_widgets.h"
#include <QString>
#include <QHBoxLayout>
#include <QLabel>

menu_spinbox::menu_spinbox(const QString &title):QWidgetAction(NULL)
{
    QWidget *p = new QWidget(NULL);
    QHBoxLayout *hlay = new QHBoxLayout();
    QLabel *label = new QLabel();
    label->setText(title);
    hlay->addWidget(label);
    SpinBox = new QSpinBox();
    hlay->addWidget(SpinBox);
    p->setLayout(hlay);

    setDefaultWidget(p);
}


QSpinBox *menu_spinbox::ret_spinBox()
{
    return (SpinBox);
}


double_menu_spinbox::double_menu_spinbox(const QString &title):QWidgetAction(NULL)
{
    QWidget *p = new QWidget(NULL);
    QHBoxLayout *hlay = new QHBoxLayout();
    QLabel *label = new QLabel(title);
    hlay->addWidget(label);
    SpinBox = new QDoubleSpinBox();
    hlay->addWidget(SpinBox);
    p->setLayout(hlay);

    setDefaultWidget(p);
}

QDoubleSpinBox *double_menu_spinbox::ret_spinBox()
{
    return SpinBox;
}

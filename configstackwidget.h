#ifndef CONFIGSTACKWIDGET_H
#define CONFIGSTACKWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "machinesettings.h"
enum Category   {
    Mining, Smelting, Pumpjack, Refinery, Chemistry, Assembling, Rocketry
};

namespace Ui {
class ConfigStackWidget;
}

class ConfigStackWidget : public QStackedWidget
{
    Q_OBJECT

public:
    explicit ConfigStackWidget(QWidget *parent = 0);
    ~ConfigStackWidget();

private:
    Ui::ConfigStackWidget *ui;
};

#endif // CONFIGSTACKWIDGET_H

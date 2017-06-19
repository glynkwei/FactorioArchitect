#include "configstackwidget.h"
#include "ui_configstackwidget.h"

ConfigStackWidget::ConfigStackWidget(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::ConfigStackWidget)
{
    ui->setupUi(this);
    addWidget(new MiningSettings);
    addWidget(new SmeltingSettings);
    addWidget(new PumpjackSettings);
    addWidget(new RefinerySettings);
    addWidget(new ChemistrySettings);
    addWidget(new AssemblingSettings);
    addWidget(new RocketrySettings);

}

ConfigStackWidget::~ConfigStackWidget()
{
    delete ui;
}

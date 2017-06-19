#include "machinesettings.h"
#include "ui_machinesettings.h"
void setIconForLabel(QString image_name, QLabel* label)  {
    QPixmap icon(image_name);
    int w =32;
    int h =32;
    icon = icon.scaled(w,h, Qt::KeepAspectRatio);
    label->setPixmap(icon);

}
MachineSettings::MachineSettings(QWidget *parent, int _type) :
    QWidget(parent),
    ui(new Ui::MachineSettings),
    type(_type)
{
    ui->setupUi(this);
    //Radio button type layout
    selectorBox = new QGroupBox;
    typeSelectorGroup = new QButtonGroup;
    QHBoxLayout* typeGroupLayout = new QHBoxLayout;
    if (type == 1)  {
        typeSelector.push_back(new NoToggleRadioButton);
        typeSelector[0]->setMinimumWidth(50);
        typeSelectorGroup->addButton(typeSelector[0],0);
        typeGroupLayout->addWidget(typeSelector[0]);
    }
    else    {
        for (int i = 0; i < type; i++)  {
            typeSelector.push_back(new QRadioButton);
            typeSelector[i]->setMinimumWidth(50);
            typeSelectorGroup->addButton(typeSelector[i],i);
            typeGroupLayout->addWidget(typeSelector[i]);
        }
    }
    selectorBox->setLayout(typeGroupLayout);
    selectorBox->setFixedHeight(50);
    //2x2 grid, column is Intermediate/Final
    //rows are Productivity/Speed
    speedConfigTable = new QTableWidget(2,2);
    speedConfigTable->setFixedWidth(180);
    speedConfigTable->setFixedHeight(180);
    auto intColumnItem = new QTableWidgetItem;
    intColumnItem->setSizeHint(QSize(50,50));
    intColumnItem->setIcon(QIcon(":/intermediate-products.png"));
    intColumnItem->setToolTip(QString("Recipes for intermediate products will use these bonuses."));
    speedConfigTable->setHorizontalHeaderItem(0,intColumnItem);
    auto finColumnItem = new QTableWidgetItem;
    finColumnItem->setSizeHint(QSize(50,50));
    finColumnItem->setIcon(QIcon(":/production.png"));
    finColumnItem->setToolTip(QString("Recipes will use these bonuses if not listed as an intermediate product."));
    speedConfigTable->setHorizontalHeaderItem(1, finColumnItem);

    auto productivityIcon = new QTableWidgetItem;
    productivityIcon->setSizeHint(QSize(50,50));
    productivityIcon->setIcon(QIcon(":/icons/productivity-module.png"));
    productivityIcon->setToolTip(QString("The productivity bonus, in terms of percent."));
    speedConfigTable->setVerticalHeaderItem(0,productivityIcon);
    speedConfigTable->verticalHeader()->setDefaultSectionSize(50);
    speedConfigTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    auto speedIcon = new QTableWidgetItem;
    speedIcon->setSizeHint(QSize(50,50));
    speedIcon->setIcon(QIcon(":/icons/speed-module.png"));
    speedIcon->setToolTip(QString("The speed bonus, in terms of percent."));
    speedConfigTable->setVerticalHeaderItem(1,speedIcon);
    speedConfigTable->horizontalHeader()->setDefaultSectionSize(50);
    speedConfigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //set QLineEdits within table
    for (int r = 0; r < 2; r++)     {
        for (int c = 0; c < 2 ; c++)    {
            QLineEdit* lineEdit = new QLineEdit;
            auto min = std::numeric_limits<double>::min();
            auto max = std::numeric_limits<double>::max();
            auto validator = new QDoubleValidator(min,max,3);
            lineEdit->setValidator(validator);
            QObject::connect(lineEdit, SIGNAL(textEdited(QString)),
                             this, SLOT(lineTextEdited(QString)));
            lineEdit->setStyleSheet("QLineEdit { border: none }");
            lineEdit->setAlignment(Qt::AlignCenter);
            speedConfigTable->setCellWidget(r,c,lineEdit);

        }
    }

    QGridLayout* scene = new QGridLayout(this);
    scene->addWidget(selectorBox,0,0,1,type);

    scene->addWidget(speedConfigTable,1,0,1,4,Qt::AlignTop);
    this->setLayout(scene);



}
void MachineSettings::lineTextEdited(QString)   {
    emit dataChanged();
}
void MachineSettings::radioChanged()  {
    emit dataChanged();
}
void MachineSettings::checkChanged()     {
    emit dataChanged();
}

void MachineSettings:: setBonuses(double intProd, double intSpd, double defProd, double defSpd) {
    static_cast<QLineEdit*>(speedConfigTable->cellWidget(0,0))->setText(QString::number(intProd));
    static_cast<QLineEdit*>(speedConfigTable->cellWidget(1,0))->setText(QString::number(intSpd));
    static_cast<QLineEdit*>(speedConfigTable->cellWidget(0,1))->setText(QString::number(defProd));
    static_cast<QLineEdit*>(speedConfigTable->cellWidget(1,1))->setText(QString::number(defSpd));
}
machine_config MachineSettings::getMachineConfig()  {
 machine_config c;
 c.intermediate_products_boost.effectivity = static_cast<QLineEdit*>(speedConfigTable->cellWidget(0,0))->text().toDouble();
 c.intermediate_products_boost.speed = static_cast<QLineEdit*>(speedConfigTable->cellWidget(1,0))->text().toDouble();
 c.default_boosts.effectivity = static_cast<QLineEdit*>(speedConfigTable->cellWidget(0,1))->text().toDouble();
 c.default_boosts.effectivity = static_cast<QLineEdit*>(speedConfigTable->cellWidget(1,1))->text().toDouble();
 c.type = typeSelectorGroup->checkedId();

 return c;
}

void MachineSettings::setIconForType(QString path, int i)  {
    QIcon icon(path);
    typeSelector[i]->setIcon(icon);
}

MachineSettings::~MachineSettings()
{
    delete ui;
}


MiningSettings::MiningSettings(QWidget* parent):
MachineSettings(parent,2)
{
    setIconForType(tr(":/icons/burner-mining-drill"),0);
    setIconForType(tr(":/icons/electric-mining-drill"),1);
    typeSelector[1]->setChecked(true);
}
SmeltingSettings::SmeltingSettings(QWidget *parent):
MachineSettings(parent,3)
{
    setIconForType(tr(":/icons/stone-furnace"),0);
    setIconForType(tr(":/icons/steel-furnace"),1);
    setIconForType(tr(":/icons/electric-furnace"),2);
    typeSelector[2]->setChecked(true);
}
PumpjackSettings::PumpjackSettings(QWidget *parent):
MachineSettings(parent,1)
{
    this->setIconForType(tr(":/icons/pumpjack"),0);
    typeSelector[0]->setChecked(true);
    oilRate = new QLineEdit;
    QObject::connect(oilRate, SIGNAL(textEdited(QString)),
                     this, SLOT(lineTextEdited(QString)));
    auto validator = new QDoubleValidator(0, std::numeric_limits<double>::max(),3);
    oilRate->setValidator(validator);
    QGridLayout* scene =static_cast<QGridLayout*>(this->layout());
    QHBoxLayout* oilAndLabel = new QHBoxLayout;

    QLabel* oilIcon = new QLabel;
    oilIcon->setToolTip(tr("The amount of oil extracted per second from each pumpjack"));
    setIconForLabel(tr(":/icons/crude-oil"),oilIcon);
    oilAndLabel->addWidget(oilIcon,0, Qt::AlignRight);
    oilAndLabel->addWidget(oilRate,0,Qt::AlignRight);

    scene->addLayout(oilAndLabel,0,3,1,1);
}
RefinerySettings::RefinerySettings(QWidget* parent):
MachineSettings(parent,1)
{
    typeSelector[0]->setChecked(true);
    processSelectorGroup = new QButtonGroup;
    QHBoxLayout* layout = new QHBoxLayout;
    processSelector = new QGroupBox;
    processSelector->setFixedHeight(50);
    setIconForType(tr(":/icons/oil-refinery"),0);
    for (int i = 0; i < 3; i++)     {
        processes.push_back(new QRadioButton);
        QObject::connect(processes[i], SIGNAL(pressed()),
                         this, SLOT(radioChanged()));
        processes[i]->setMinimumWidth(50);
        processSelectorGroup->addButton(processes[i],i);
        layout->insertWidget(i,processes[i]);

    }
    processes[0]->setIcon(QIcon(":/icons/basic-oil-processing"));
    processes[1]->setIcon(QIcon(":/icons/advanced-oil-processing"));
    processes[2]->setIcon(QIcon(":/icons/coal-liquefaction"));
    processes[1]->setChecked(true);

    processSelector->setLayout(layout);
    QGridLayout* scene =static_cast<QGridLayout*>(this->layout());
    scene->addWidget(processSelector,0,1,1,3);

    crackBox = new QCheckBox;
    QObject::connect(crackBox, SIGNAL(pressed()), this, SLOT(checkChanged()));
    crackBox->setToolTip(tr("Enable cracking whenever possible. If this is disabled, there may be excessive fluid byproducts."));
    crackBox->setIcon(QIcon(":/icons/light-oil-cracking-to-petroleum-gas"));
    crackBox->setChecked(true);
    scene->addWidget(crackBox,2,0,1,1);

}
ChemistrySettings::ChemistrySettings(QWidget* parent):
MachineSettings(parent,1)
{
    typeSelector[0]->setChecked(true);
    fuelSelectorGroup = new QButtonGroup;
    setIconForType(tr(":/icons/chemical-plant"),0);
    fuelSelector = new QGroupBox;
    fuelSelector->setFixedHeight(50);
    fuelSelector->setToolTip(tr("The selected fluid for the solid fuel recipe"));
    QHBoxLayout* layout = new QHBoxLayout;
    for (int i = 0; i < 3; i++)     {
        fuels.push_back(new QRadioButton);
        fuels[i]->setMinimumWidth(50);
        QObject::connect(fuels[i], SIGNAL(pressed()),
                         this, SLOT(radioChanged()));
        fuelSelectorGroup->addButton(fuels[i],i);
        layout->insertWidget(i,fuels[i]);

    }
    fuelSelector->setLayout(layout);
    fuels[0]->setIcon(QIcon(":/icons/heavy-oil"));
    fuels[1]->setIcon(QIcon(":/icons/light-oil"));
    fuels[2]->setIcon(QIcon(":/icons/petroleum-gas"));
    fuels[2]->setChecked(true);
    QGridLayout* scene =static_cast<QGridLayout*>(this->layout());
    scene->addWidget(fuelSelector,0,1,1,3);

}
AssemblingSettings::AssemblingSettings(QWidget *parent):
MachineSettings(parent,3)
{
    typeSelector[2]->setChecked(true);
    setIconForType(tr(":/icons/assembling-machine-1"),0);
    setIconForType(tr(":/icons/assembling-machine-2"),1);
    setIconForType(tr(":/icons/assembling-machine-3"),2);
}
RocketrySettings::RocketrySettings(QWidget *parent):
MachineSettings(parent,1)
{
    typeSelector[0]->setChecked(true);
    setIconForType(tr(":/icons/rocket-silo"),0);
}

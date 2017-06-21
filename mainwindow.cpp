#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextStream>
#include <QFile>

using namespace std;
using json = nlohmann::json;


void setIconForLabel(string image_name, QLabel* label)  {
    QPixmap icon(QString::fromStdString(image_name));
    int w =32;
    int h =32;
    icon = icon.scaled(w,h, Qt::KeepAspectRatio);
    label->setPixmap(icon);

}


void insertIntoTableWidget(QTableWidget* table, MainWindow* w)   {
    int total_rows = table->rowCount();
    table->insertRow(total_rows);
    auto checkbox = new QCheckBox(table);
    checkbox->setChecked(true);
    checkbox->setStyleSheet("margin-left:25%; margin-right:25%;");
    QObject::connect(checkbox, SIGNAL(stateChanged(int)), w, SLOT(checkbox_stateChanged(int)));

    auto lineEdit = new QLineEdit(table);
    lineEdit->setStyleSheet("QLineEdit { border: none }");
    double min = numeric_limits<double>::min();
    double max = numeric_limits<double>::max();
    auto validator = new QDoubleValidator(min,max,3,table);
    validator->setNotation(QDoubleValidator::Notation::StandardNotation);
    lineEdit->setValidator(validator);
    QObject::connect(lineEdit, SIGNAL(textEdited(QString)),
                    w, SLOT(lineEdit_textEdited(QString)));



    auto icon = new QLabel(table);
    table->setCellWidget(total_rows,0,icon);
    table->setCellWidget(total_rows,1,lineEdit);
    table->setCellWidget(total_rows,2, checkbox);

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    dialog(nullptr),
    ui(new Ui::MainWindow)
{
    //set up
    QFile file(":/data01513.json");
    file.open(  QFile::ReadOnly );
    QTextStream stream(&file);
    std::string str = stream.readAll().toStdString();
    data = nlohmann::json::parse(str);

    ui->setupUi(this);
    createInventorySearchDialog();
    ui->ordersTable->setColumnWidth(0,100);
    ui->ordersTable->setColumnWidth(2,80);
    ui->ordersTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    ui->ordersTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->ordersTable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);

    ui->resultTable->setColumnWidth(0,100);
    ui->resultTable->header()->setSectionResizeMode(0,QHeaderView::Fixed);
    ui->resultTable->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->resultTable->header()->setSectionResizeMode(2, QHeaderView::Stretch);

    //SelectPage
    ui->configWidget->setCurrentIndex(0);
    for (int i = 0; i < 7; i++)     {
        QObject::connect(static_cast<MachineSettings*>(ui->configWidget->widget(i)), SIGNAL(dataChanged()),
                         this, SLOT(configWidget_dataChanged()));
    }
    //Set icons for combobox
    ui->selectPageWidget->setItemIcon(0, QIcon(tr(":/icons/electric-mining-drill")));
    ui->selectPageWidget->setItemIcon(1, QIcon(tr(":/icons/electric-furnace")));
    ui->selectPageWidget->setItemIcon(2, QIcon(tr(":/icons/pumpjack")));
    ui->selectPageWidget->setItemIcon(3, QIcon(tr(":/icons/oil-refinery")));
    ui->selectPageWidget->setItemIcon(4, QIcon(tr(":/icons/chemical-plant")));
    ui->selectPageWidget->setItemIcon(5, QIcon(tr(":/icons/assembling-machine-3")));
    ui->selectPageWidget->setItemIcon(6, QIcon(tr(":/icons/rocket-silo")));
    //validate all line edits

    //Order Table
    ui->ordersTable->horizontalScrollBar()->setEnabled(false);
    insertIntoTableWidget(ui->ordersTable,this);

    readConfig();

    loadResultTable();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readConfig()   {

    string file_name = "config.json";
    ifstream in(file_name);
    json j;
    try     {
        in >> j;
    }
    catch (exception e)     {
        in.close();
        return;
    }

    in.close();

    vector<order> history;
    set<string> items_to_share;
    production_config config;
    try     {
        history = j.at("orders").get<vector<order>>();
        items_to_share = j.at("sharedItem").get<set<string>>();
        config = j.at("config").get<production_config>();
    }
    catch (json::type_error&)    {
        return;
    }
    catch(json::out_of_range&)     {
        return;
    }

    //Config: set view
    auto miningSettings = static_cast<MiningSettings*>(ui->configWidget->widget(0));
    miningSettings->typeSelector[config.mining_config.type]->setChecked(true);

    auto smeltingSettings = static_cast<SmeltingSettings*>(ui->configWidget->widget(1));
    smeltingSettings->typeSelector[config.smelting_config.type]->setChecked(true);

    auto  pumpjackSettings = static_cast<PumpjackSettings*>(ui->configWidget->widget(2));
    pumpjackSettings->typeSelector[config.pumpjack_config.type]->setChecked(true);
    pumpjackSettings->oilRate->setText(QString::number(config.oil_extraction_rate));

    auto refinerySettings = static_cast<RefinerySettings*>(ui->configWidget->widget(3));
    refinerySettings->typeSelector[config.refinery_config.type]->setChecked(true);
    refinerySettings->crackBox->setChecked(config.crack_when_possible);
    refinerySettings->processes[config.ref_type]->setChecked(true);

    auto chemistrySettings = static_cast<ChemistrySettings*>(ui->configWidget->widget(4));
    chemistrySettings->typeSelector[config.chemical_config.type]->setChecked(true);
    chemistrySettings->fuels[config.fuel_type]->setChecked(true);

    auto assemblingSettings = static_cast<AssemblingSettings*>(ui->configWidget->widget(5));
    assemblingSettings->typeSelector[config.assembling_config.type]->setChecked(true);

    auto rocketrySettings = static_cast<RocketrySettings*>(ui->configWidget->widget(6));
    rocketrySettings->typeSelector[config.rocket_config.type]->setChecked(true);

    miningSettings->setBonuses(config.mining_config.intermediate_products_boost.effectivity,
                               config.mining_config.intermediate_products_boost.speed,
                               config.mining_config.default_boosts.effectivity,
                               config.mining_config.default_boosts.speed);

    smeltingSettings->setBonuses(config.smelting_config.intermediate_products_boost.effectivity,
                               config.smelting_config.intermediate_products_boost.speed,
                               config.smelting_config.default_boosts.effectivity,
                               config.smelting_config.default_boosts.speed);

    pumpjackSettings->setBonuses(config.pumpjack_config.intermediate_products_boost.effectivity,
                               config.pumpjack_config.intermediate_products_boost.speed,
                               config.pumpjack_config.default_boosts.effectivity,
                               config.pumpjack_config.default_boosts.speed);

    refinerySettings->setBonuses(config.refinery_config.intermediate_products_boost.effectivity,
                               config.refinery_config.intermediate_products_boost.speed,
                               config.refinery_config.default_boosts.effectivity,
                               config.refinery_config.default_boosts.speed);

    chemistrySettings->setBonuses(config.chemical_config.intermediate_products_boost.effectivity,
                               config.chemical_config.intermediate_products_boost.speed,
                               config.chemical_config.default_boosts.effectivity,
                               config.chemical_config.default_boosts.speed);

    assemblingSettings->setBonuses(config.assembling_config.intermediate_products_boost.effectivity,
                               config.assembling_config.intermediate_products_boost.speed,
                               config.assembling_config.default_boosts.effectivity,
                               config.assembling_config.default_boosts.speed);

    rocketrySettings->setBonuses(config.rocket_config.intermediate_products_boost.effectivity,
                               config.rocket_config.intermediate_products_boost.speed,
                               config.rocket_config.default_boosts.effectivity,
                               config.rocket_config.default_boosts.speed);




    //History
    for (int i = 0; i < history.size(); i++)    {
        insertIntoTableWidget(ui->ordersTable,this);
    }
    //Shared Items
    for (int i = 0; i < history.size(); i++)    {
        item_selected(history[i].item_name,i,0);
        static_cast<QLineEdit*>(ui->ordersTable->cellWidget(i,1))->setText(QString::number(history[i].rate));
        if (items_to_share.find(history[i].item_name) != items_to_share.end())  {
            static_cast<QCheckBox*>(ui->ordersTable->cellWidget(i,2))->setChecked(true);
        }
    }
}

 void MainWindow::createInventorySearchDialog()   {
     if (!dialog)    {
         dialog = new InventorySearchDialog(data,0,0,ui->ordersTable, Qt::WindowCloseButtonHint);
         dialog->setWindowTitle(tr("Item Finder"));
         QIcon icon((tr(":/icons/logistic-chest-requester.png")));
         dialog->setWindowIcon(icon);

         QObject::connect(&dialog->inven, SIGNAL(select_item(std::string,int,int)),
                      this, SLOT(item_selected(std::string,int,int)));
     }
 }

void MainWindow::on_ordersTable_clicked(const QModelIndex &index)
{
    int total_rows = ui->ordersTable->rowCount();
    //add a new row if we click on last row
    if (index.row() == total_rows - 1)  {
        insertIntoTableWidget(ui->ordersTable, this);
    }
    //prompt item selection if in name column
    if (index.column() == 0)    {
        //
        if (index.column() == 0)    {
            ui->ordersTable->cellWidget(index.row(),0)->setProperty("item-name",QVariant());
            static_cast<QLabel*>(ui->ordersTable->cellWidget(index.row(),0))->setPixmap(QPixmap());
            static_cast<QLabel*>(ui->ordersTable->cellWidget(index.row(),0))->setToolTip("");
        }

        int cell_x = ui->ordersTable->columnViewportPosition(index.column());
        int cell_y = ui->ordersTable->rowViewportPosition(index.row());
        QPoint global = ui->ordersTable->mapToGlobal({cell_x,cell_y});
        int cell_width = ui->ordersTable->columnWidth(index.column());

        QPoint dialog_location = {global.x()+cell_width, global.y()};

        if (!dialog)     {
            createInventorySearchDialog();
        }
        dialog->row = index.row();
        dialog->col = index.column();
        dialog->inven.reset();
        dialog->move(dialog_location);
        dialog->show();

    }
}
void MainWindow::loadResultTable()  {

    ui->resultTable->clear();
    set<string> planners_to_share;
    int row_count = ui->ordersTable->rowCount();
    for (int i = 0; i < row_count; i++)     {
        if (static_cast<QCheckBox*>(ui->ordersTable->cellWidget(i,2))->checkState() == Qt::CheckState::Checked)    {
            QString name = ui->ordersTable->cellWidget(i,0)->property("item-name").toString();
            planners_to_share.insert(name.toStdString());
        }
    }
    production_config config;
    config.mining_config = static_cast<MiningSettings*>(ui->configWidget->widget(0))->getMachineConfig();

    config.smelting_config = static_cast<SmeltingSettings*>(ui->configWidget->widget(1))->getMachineConfig();

    config.pumpjack_config = static_cast<PumpjackSettings*>(ui->configWidget->widget(2))->getMachineConfig();
    config.oil_extraction_rate = static_cast<PumpjackSettings*>(ui->configWidget->widget(2))->oilRate->text().toDouble();
    config.refinery_config = static_cast<RefinerySettings*>(ui->configWidget->widget(3))->getMachineConfig();
    config.crack_when_possible = static_cast<RefinerySettings*>(ui->configWidget->widget(3))->crackBox->checkState();
    config.ref_type = (refinery_type)static_cast<RefinerySettings*>(ui->configWidget->widget(3))->processSelectorGroup->checkedId();
    config.chemical_config = static_cast<ChemistrySettings*>(ui->configWidget->widget(4))->getMachineConfig();
    config.fuel_type = (solid_fuel_type)static_cast<ChemistrySettings*>(ui->configWidget->widget(4))->fuelSelectorGroup->checkedId();
    config.assembling_config = static_cast<AssemblingSettings*>(ui->configWidget->widget(5))->getMachineConfig();

    config.rocket_config = static_cast<RocketrySettings*>(ui->configWidget->widget(6))->getMachineConfig();


    factory fy(config,data,planners_to_share);

    for (int i = 0; i < row_count; i++)   {
        QString name = ui->ordersTable->cellWidget(i,0)->property("item-name").toString();
        auto rate =  static_cast<QLineEdit*>(ui->ordersTable->cellWidget(i,1))->text().toDouble();
        if (!name.isEmpty() && rate != 0)   {
            fy.plan(name.toStdString(),rate);
        }
    }

    //done with planning, save everything
    ofstream out("config.json");
    out << fy.serialize();
    out.close();

    for (auto planner : fy.component_planners)    {
        auto component = new QTreeWidgetItem(ui->resultTable);
        string file_path = ":/icons/"+planner.first;
        component->setIcon(0,QIcon(QString::fromStdString(file_path)));
        component->setSizeHint(0, {32,32});
        component->setToolTip(0, QString::fromStdString(planner.first));
        for (auto entry : planner.second)    {
            auto recipe = new QTreeWidgetItem(component);
            string file_path = ":/icons/"+entry.first;
            recipe->setIcon(0,QIcon(QString::fromStdString(file_path)));
            recipe->setSizeHint(0, {32,32});
            recipe->setToolTip(0,QString::fromStdString(entry.first));
            recipe->setToolTip(1,QString::fromStdString(entry.first));
            recipe->setToolTip(2,QString::fromStdString(entry.first));
            recipe->setText(1, QString::number(entry.second.product, 'f',3));
            if (entry.second.count != -1)   {
                if (entry.second.type == "mining")  {
                    if (config.mining_config.type == 0)     {
                        recipe->setIcon(2, QIcon(":/icons/burner-mining-drill.png"));
                    }
                    else if (config.mining_config.type == 1)    {
                        recipe->setIcon(2, QIcon(":/icons/electric-mining-drill.png"));
                    }
                }
                else if (entry.second.type == "smelting")    {
                    if (config.smelting_config.type == 0)   {
                        recipe->setIcon(2, QIcon(":/icons/stone-furnace.png"));
                    }
                    else if (config.smelting_config.type == 1)  {
                        recipe->setIcon(2, QIcon(":/icons/steel-furnace.png"));
                    }
                    else if (config.smelting_config.type == 2)  {
                        recipe->setIcon(2, QIcon(":/icons/electric-furnace.png"));
                    }
                }
                else if (entry.second.type == "pumpjack")   {
                    if (config.pumpjack_config.type == 0)   {
                        recipe->setIcon(2, QIcon(":/icons/pumpjack.png"));
                    }
                }
                else if (entry.second.type == "refinery")   {
                    if (config.refinery_config.type == 0)   {
                        recipe->setIcon(2, QIcon(":/icons/oil-refinery.png"));
                    }
                }
                else if (entry.second.type == "chemistry")  {
                    if (config.chemical_config.type == 0)   {
                        recipe->setIcon(2, QIcon(":/icons/chemical-plant.png"));
                    }
                }
                else if (entry.second.type == "assembling")     {
                    if (config.assembling_config.type ==0)  {
                        recipe->setIcon(2, QIcon(":/icons/assembling-machine-1.png"));
                    }
                    else if (config.assembling_config.type == 1)    {
                        recipe->setIcon(2, QIcon(":/icons/assembling-machine-2.png"));
                    }
                    else if (config.assembling_config.type == 2)    {
                        recipe->setIcon(2, QIcon(":/icons/assembling-machine-3.png"));
                    }
                }
                else if (entry.second.type == "rocketry")   {
                    if (config.rocket_config.type == 0)     {
                        recipe->setIcon(2, QIcon(":/icons/rocket-silo.png"));
                    }
                }
                else if (entry.second.type == "offshore-pump")  {
                    recipe->setIcon(2,QIcon(":/icons/offshore-pump.png"));
                }
                else if (entry.second.type == "boiler")     {
                    recipe->setIcon(2,QIcon(":/icons/boiler.png"));
                }
                recipe->setText(2, QString::number(entry.second.count,'f',3));
            }
        }
    }
    auto root = new QTreeWidgetItem(ui->resultTable);
    root->setIcon(0,QIcon(QString::fromStdString(":/blue_infinity_transparent.png")));
    root->setToolTip(0, "Shared");
    root->setSizeHint(0, {32,32});
    for (auto entry : fy.shared_planner)     {
        auto recipe = new QTreeWidgetItem(root);
        string file_path = ":/icons/"+entry.first;
        recipe->setIcon(0,QIcon(QString::fromStdString(file_path)));
        recipe->setSizeHint(0, {32,32});
        recipe->setToolTip(0,QString::fromStdString(entry.first));
        recipe->setToolTip(1,QString::fromStdString(entry.first));
        recipe->setToolTip(2,QString::fromStdString(entry.first));
        recipe->setText(1, QString::number(entry.second.product,'f',3));
        if (entry.second.count != -1)   {
            if (entry.second.type == "mining")  {
                if (config.mining_config.type == 0)     {
                    recipe->setIcon(2, QIcon(":/icons/burner-mining-drill.png"));
                }
                else if (config.mining_config.type == 1)    {
                    recipe->setIcon(2, QIcon(":/icons/electric-mining-drill.png"));
                }
            }
            else if (entry.second.type == "smelting")    {
                if (config.smelting_config.type == 0)   {
                    recipe->setIcon(2, QIcon(":/icons/stone-furnace.png"));
                }
                else if (config.smelting_config.type == 1)  {
                    recipe->setIcon(2, QIcon(":/icons/steel-furnace.png"));
                }
                else if (config.smelting_config.type == 2)  {
                    recipe->setIcon(2, QIcon(":/icons/electric-furnace.png"));
                }
            }
            else if (entry.second.type == "pumpjack")   {
                if (config.pumpjack_config.type == 0)   {
                    recipe->setIcon(2, QIcon(":/icons/pumpjack.png"));
                }
            }
            else if (entry.second.type == "refinery")   {
                if (config.refinery_config.type == 0)   {
                    recipe->setIcon(2, QIcon(":/icons/oil-refinery.png"));
                }
            }
            else if (entry.second.type == "chemistry")  {
                if (config.chemical_config.type == 0)   {
                    recipe->setIcon(2, QIcon(":/icons/chemical-plant.png"));
                }
            }
            else if (entry.second.type == "assembling")     {
                if (config.assembling_config.type ==0)  {
                    recipe->setIcon(2, QIcon(":/icons/assembling-machine-1.png"));
                }
                else if (config.assembling_config.type == 1)    {
                    recipe->setIcon(2, QIcon(":/icons/assembling-machine-2.png"));
                }
                else if (config.assembling_config.type == 2)    {
                    recipe->setIcon(2, QIcon(":/icons/assembling-machine-3.png"));
                }
            }
            else if (entry.second.type == "rocketry")   {
                if (config.rocket_config.type == 0)     {
                    recipe->setIcon(2, QIcon(":/icons/rocket-silo.png"));
                }
            }
            else if (entry.second.type == "offshore-pump")  {
                recipe->setIcon(2,QIcon(":/icons/offshore-pump.png"));
            }
            else if (entry.second.type == "boiler")     {
                recipe->setIcon(2,QIcon(":/icons/boiler.png"));
            }
            recipe->setText(2, QString::number(entry.second.count,'f',3));
        }
    }
    ui->resultTable->expandAll();
    ui->resultTable->repaint();
}

void MainWindow::on_selectPageWidget_activated(const QString &arg1)
{

    if (arg1 == "Mining")   {
        ui->configWidget->setCurrentIndex(0);
    }
    else if (arg1 == "Smelting")    {
        ui->configWidget->setCurrentIndex(1);
    }
    else if (arg1 == "Pumpjack")    {
        ui->configWidget->setCurrentIndex(2);
    }
    else if (arg1 == "Refinery")    {
        ui->configWidget->setCurrentIndex(3);
    }
    else if (arg1 == "Chemistry")    {
        ui->configWidget->setCurrentIndex(4);
    }
    else if (arg1 == "Assembling")    {
        ui->configWidget->setCurrentIndex(5);
    }
    else if (arg1 == "Rocketry")    {
        ui->configWidget->setCurrentIndex(6);
    }
    else    {
        throw 0;
    }
}
void MainWindow::lineEdit_textEdited(QString)     {
    loadResultTable();
}
void MainWindow::checkbox_stateChanged(int)  {
    loadResultTable();
}

void MainWindow::item_selected(string item, int r, int c)  {
    string file_path = ":/icons/"+item;
    auto icon = static_cast<QLabel*>(ui->ordersTable->cellWidget(r,c));
    setIconForLabel(file_path, icon);
    icon->setAlignment(Qt::AlignCenter);
    icon->setToolTip(QString::fromStdString(item));
    icon->setProperty("item-name", QString::fromStdString(item));
    loadResultTable();
}

void MainWindow::configWidget_dataChanged()  {
    loadResultTable();
}







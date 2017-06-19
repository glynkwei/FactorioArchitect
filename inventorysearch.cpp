#include "inventorysearch.h"
#include "inventorysearchdialog.h"
#include "ui_inventorysearch.h"
#include "subseq.h"
using namespace std;
using json = nlohmann::json;
void _setIconForLabel(string image_name, QLabel* label)  {
    QPixmap icon(QString::fromStdString(image_name));
    int w =32;
    int h =32;
    icon = icon.scaled(w,h, Qt::KeepAspectRatio);
    label->setPixmap(icon);
}
InventorySearch::InventorySearch(json &dat,QWidget *parent) :
    QWidget(parent),
    data(dat),
    ui(new Ui::InventorySearch)
{
    ui->setupUi(this);
    ui->exitButton->setFixedSize(QSize(32,32));
    ui->exitButton->setIcon(QIcon(tr(":/exit")));
    json items = data["item"];
    json fluids = data["fluid"];
    json modules = data["module"];
    json ammo = data["ammo"];
    json recipe = data["recipe"];
    auto guns = {"combat-shotgun",
                 "flamethrower",
                 "pistol",
                 "rocket-launcher",
                 "shotgun",
                 "submachine-gun"};
    set<string> forbidden = {"electric-energy-interface",
                             "simple-entity-with-force",
                             "simple-entity-with-owner",
                             "player-port",
                             "fast-loader",
                             "express-loader",
                             "loader",
                             "railgun",
                             "railgun-dart",
                             "coin",
                             "small-plane",
                             "solid-fuel-from-heavy-oil",
                             "solid-fuel-from-light-oil",
                             "solid-fuel-from-petroleum-gas",
                             "nuclear-fuel-reprocessing",
                             "kovarex-enrichment-process",
                             "heavy-oil-cracking",
                             "light-oil-cracking",
                             "coal-liquefaction",
                             "advanced-oil-processing",
                             "basic-oil-processing"};




    for (json::iterator it = recipe.begin(); it != recipe.end(); ++it)     {
        string key = it.key();
        if (forbidden.find(key) == forbidden.end())     {
            itemNames.insert(key);
        }
    }
    for (json::iterator it = fluids.begin(); it != fluids.end(); ++it)   {
        string key = it.key();
        if (forbidden.find(key) == forbidden.end())     {
            itemNames.insert(key);
        }
    }
    int size = itemNames.size();
    for (int i = 0; i < size/10 +1;i++)     {
        ui->inventoryTable->insertRow(i);
    }
    for (int r = 0; r < ui->inventoryTable->rowCount(); r++)    {
        for (int c =0; c < ui->inventoryTable->columnCount();c++)   {
            auto icon = new QLabel(ui->inventoryTable);
            ui->inventoryTable->setCellWidget(r,c,icon);
        }
    }
    _setIconForLabel(":/search.png",ui->searchIcon);
    loadAssets();
}

InventorySearch::~InventorySearch()
{
    delete ui;
}
void InventorySearch::loadAssets(string key)  {
    for (char &c : key)     {
        if (c == ' ')   {
            c = '-';
        }
        c = tolower(c);
    }
    int width = ui->inventoryTable->columnCount();
    int i = 0;
    set<string> supersequences;
    matchSubsequence(key,itemNames,supersequences);

    for (auto name : supersequences)  {
        auto icon = static_cast<QLabel*>(ui->inventoryTable->cellWidget(i/width,i%width));
        icon->setProperty("item-name",QString::fromStdString(name));
        string file_path = ":/icons/"+name;
        _setIconForLabel(file_path, icon);
        i++;
    }
    int total_cells = ui->inventoryTable->rowCount()*ui->inventoryTable->columnCount();
    for (int j = i; j < total_cells; j++)   {
        auto icon = static_cast<QLabel*>(ui->inventoryTable->cellWidget(j/width,j%width));
        icon->setPixmap(QPixmap());
    }
   ui->inventoryTable->scrollToTop();
}

void InventorySearch::on_inventoryTable_activated(const QModelIndex &index)
{
    string item = static_cast<QLabel*>(ui->inventoryTable->cellWidget(index.row(),index.column()))->property("item-name").toString().toStdString();
    auto dialog = static_cast<InventorySearchDialog*>(this->parentWidget());
    int r = dialog->row;
    int c = dialog->col;
    select_item(item,r,c);
    dialog->hide();
}

void InventorySearch::on_lineSearch_textChanged(const QString &arg1)
{
    loadAssets(arg1.toStdString());
}

void InventorySearch::on_inventoryTable_clicked(const QModelIndex &index)
{
    string item = static_cast<QLabel*>(ui->inventoryTable->cellWidget(index.row(),index.column()))->property("item-name").toString().toStdString();
    auto dialog = static_cast<InventorySearchDialog*>(this->parentWidget());
    int r = dialog->row;
    int c = dialog->col;
    emit select_item(item,r,c);
    dialog->hide();
}
void InventorySearch::reset()   {
    ui->lineSearch->setText(tr(""));
}

void InventorySearch::on_exitButton_clicked()
{
    auto dialog = static_cast<InventorySearchDialog*>(this->parentWidget());
    dialog->hide();
}


#ifndef INVENTORYSEARCH_H
#define INVENTORYSEARCH_H
#include "json.hpp"
#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPainter>
#include <QBrush>
#include <set>
namespace Ui {
class InventorySearch;
}

class InventorySearch : public QWidget
{
    Q_OBJECT

public:
    explicit InventorySearch(nlohmann::json &dat, QWidget *parent = 0);
    void reset();
    ~InventorySearch();
signals:
    void select_item(std::string item, int r, int c);
private slots:
    void on_inventoryTable_activated(const QModelIndex &index);

    void on_lineSearch_textChanged(const QString &arg1);

    void on_inventoryTable_clicked(const QModelIndex &index);

    void on_exitButton_clicked();

private:
    Ui::InventorySearch *ui;
    nlohmann::json &data;
    std::set<std::string> itemNames;
    void loadAssets(std::string key = "");
};

#endif // INVENTORYSEARCH_H

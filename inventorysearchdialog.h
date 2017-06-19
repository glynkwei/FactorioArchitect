#ifndef INVENTORYSEARCHDIALOG_H
#define INVENTORYSEARCHDIALOG_H
#include <QDialog>
#include "inventorysearch.h"

class InventorySearchDialog : public QDialog
{
public:
    InventorySearch inven;
    int row, col;
    InventorySearchDialog(nlohmann::json &dat, int r, int c,QWidget* parent = 0, Qt::WindowFlags f = 0);

};

#endif // INVENTORYSEARCHDIALOG_H

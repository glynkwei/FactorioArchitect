#include "inventorysearchdialog.h"

InventorySearchDialog::InventorySearchDialog(nlohmann::json &dat,int r, int c,QWidget * parent, Qt::WindowFlags f): QDialog(parent, f), inven(dat,this), row(r), col(c)   {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    inven.show();
}

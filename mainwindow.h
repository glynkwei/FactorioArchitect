#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "perfectio.h"
#include "inventorysearch.h"
#include "inventorysearchdialog.h"
#include "configstackwidget.h"
#include <QMainWindow>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QDialog>
#include <QDoubleValidator>
#include <QWidget>
#include <QScrollBar>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:
    void on_ordersTable_clicked(const QModelIndex &index);


    void on_selectPageWidget_activated(const QString &arg1);

    void item_selected(std::string item,int r, int c);

    void lineEdit_textEdited(QString);

    void checkbox_stateChanged(int);

    void configWidget_dataChanged();


private:
    nlohmann::json data;
    Ui::MainWindow *ui;
    InventorySearchDialog* dialog;
    void readConfig();
    void loadResultTable();
    void createInventorySearchDialog();

};



#endif // MAINWINDOW_H

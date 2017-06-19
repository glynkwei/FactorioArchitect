#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QStyle>
#include <QTextStream>
using namespace std;
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    QFile stylesheet(":/stylesheet.qss");
    stylesheet.open(QFile::ReadOnly);
    QTextStream stream(&stylesheet);
    auto sheetString = stream.readAll();
    w.setStyleSheet(sheetString);
    return a.exec();


}

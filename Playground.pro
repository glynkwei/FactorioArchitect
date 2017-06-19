#-------------------------------------------------
#
# Project created by QtCreator 2017-05-22T18:10:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Playground
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    perfectio.cpp \
    notoggleradiobutton.cpp \
    inventorysearch.cpp \
    inventorysearchdialog.cpp \
    machinesettings.cpp \
    configstackwidget.cpp

HEADERS  += mainwindow.h \
    json.hpp \
    perfectio.h \
    notoggleradiobutton.h \
    inventorysearch.h \
    inventorysearchdialog.h \
    subseq.h \
    machinesettings.h \
    configstackwidget.h

FORMS    += mainwindow.ui \
    inventorysearch.ui \
    machinesettings.ui \
    configstackwidget.ui

RESOURCES += \
    resource.qrc

RC_FILE = perfectio.rc

QMAKE_RESOURCE_FLAGS += -compress 0

DISTFILES +=

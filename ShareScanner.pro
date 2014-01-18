#-------------------------------------------------
#
# Project created by QtCreator 2012-08-19T13:04:50
#
#-------------------------------------------------

QT       += core gui
QT       += network
CONFIG   += qtestlib

TARGET = ShareScanner
TEMPLATE = app


SOURCES += main.cpp\
        sharescanner.cpp \
    aboutdialog.cpp

HEADERS  += \
    sharescanner.h \
    aboutdialog.h

FORMS    += sharescanner.ui \
    aboutdialog.ui

RESOURCES += SSres.qrc

unix:!macx:!symbian: LIBS += -lsmbclient

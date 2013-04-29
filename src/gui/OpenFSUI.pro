#-------------------------------------------------
#
# Project created by QtCreator 2013-04-23T18:16:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenFSUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    localclient.cpp \
    info.cpp

HEADERS  += mainwindow.h \
    localclient.h \
    info.h

FORMS    += mainwindow.ui \
    localclient.ui


INCLUDEPATH += ../

#-------------------------------------------------
#
# Project created by QtCreator 2013-04-23T18:16:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenFSUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    localclient.cpp \
    info.cpp \
    ../../build/src/messages.pb.cc \
    connection.cpp \
    connectiondialog.cpp

HEADERS  += mainwindow.h \
    localclient.h \
    info.h \
    ../../build/src/messages.pb.h \
    connection.h \
    connectiondialog.h

FORMS    += mainwindow.ui \
    localclient.ui


INCLUDEPATH += ../ \
    ../../build/src/ \
    /usr/local/include/ \
    /usr/local/include/google/


unix|win32: LIBS += \
    -lcrypto++ \
    -lprotobuf

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
    /home/gabe/Code/build/openbookfs/src/messages.pb.cc \
    connection.cpp \


HEADERS  += mainwindow.h \
    localclient.h \
    info.h \
    /home/gabe/Code/build/openbookfs/src/messages.pb.h \
    connection.h \

FORMS    += mainwindow.ui \
    localclient.ui


INCLUDEPATH += ../ \
    /home/gabe/Code/build/openbookfs/src/ \
    /home/gabe/Code/openbookfs/src/ \
    /usr/local/include/ \
    /usr/local/include/google/


unix|win32: LIBS += \
    -lcrypto++ \
    -lprotobuf

OTHER_FILES += \
    ../messages.proto \
    ../CMakeLists.txt \
    ../backend/config.yaml \
    ../backend/CMakeLists.txt

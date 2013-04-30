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
    info.cpp \
    ../TimeVal.cpp \
    ../TimeSpec.cpp \
    ../SelectSpec.cpp \
    ../NotifyPipe.cpp \
    ../messages.cpp \
    ../Marshall.cpp \
    ../FileDescriptor.cpp \
    ../FdSet.cpp \
    ../msg_gen/MessageStr.cpp \
    ../../../builds/openbookfs/src/messages.pb.cc

HEADERS  += mainwindow.h \
    localclient.h \
    info.h \
    ../TimeVal.h \
    ../TimeSpec.h \
    ../SelectSpec.h \
    ../ReferenceCounted.h \
    ../NotifyPipe.h \
    ../messages.h \
    ../Marshall.h \
    ../FileDescriptor.h \
    ../FdSet.h \
    ../ExceptionStream.h \
    ../msg_gen/MessageStr.h \
    ../msg_gen/MessageMap.h \
    ../msg_gen/MessageId.h \
    global.h \
    ../../../builds/openbookfs/src/messages.pb.h

FORMS    += mainwindow.ui \
    localclient.ui


INCLUDEPATH += ../ \
    /home/josh/Codes/cpp/builds/openbookfs/src/ \
    /home/josh/devroot/usr/include/ \
    /usr/local/include/google/


unix|win32: LIBS += \
    -lcrypto++ \
    -lprotobuf

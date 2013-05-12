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
    Options.cpp \
    commands/StartSync.cpp \
    commands/SetRemoteSocket.cpp \
    commands/SetLocalSocket.cpp \
    commands/SetDisplayName.cpp \
    commands/SetClientSocket.cpp \
    commands/SaveConfig.cpp \
    commands/LoadConfig.cpp \
    commands/ListMounts.cpp \
    commands/ListKnownPeers.cpp \
    commands/Connect.cpp \
    ../TimeVal.cpp \
    ../TimeSpec.cpp \
    ../SelectSpec.cpp \
    ../NotifyPipe.cpp \
    ../messages.cpp \
    ../MessageBuffer.cpp \
    ../Marshall.cpp \
    ../FileDescriptor.cpp \
    ../FdSet.cpp \
    ../base64.cpp


HEADERS  += mainwindow.h \
    localclient.h \
    info.h \
    /home/gabe/Code/build/openbookfs/src/messages.pb.h \
    connection.h \
    Options.h \
    global.h \
    commands/UnmountOptions.h \
    commands/StartSync.h \
    commands/SetRemoteSocket.h \
    commands/SetLocalSocket.h \
    commands/SetDisplayName.h \
    commands/SetDataDir.h \
    commands/SetClientSocket.h \
    commands/SaveConfig.h \
    commands/MaxConnectionsOptions.h \
    commands/LoadConfig.h \
    commands/ListMounts.h \
    commands/ListKnownPeers.h \
    commands/Connect.h \
    ../TimeVal.h \
    ../TimeSpec.h \
    ../Synchronized.h \
    ../SelectSpec.h \
    ../ReferenceCounted.h \
    ../Queue.h \
    ../PriorityQueue.h \
    ../Pool.h \
    ../NotifyPipe.h \
    ../messages.h \
    ../MessageBuffer.h \
    ../Marshall.h \
    ../FileDescriptor.h \
    ../FdSet.h \
    ../ExceptionStream.h \
    ../Bytes.h \
    ../base64.h

FORMS    += mainwindow.ui \
    localclient.ui


INCLUDEPATH += ../ \
    /home/gabe/Code/build/openbookfs/src/ \
    /home/gabe/Code/openbookfs/src/ \
    /usr/local/include/ \
    /usr/local/include/google/


unix|win32: LIBS += \
    -lcrypto++ \
    -lprotobuf \
    -lboost_system

OTHER_FILES += \
    ../messages.proto \
    ../CMakeLists.txt


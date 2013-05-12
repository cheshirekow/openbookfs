#include "connection.h"

using namespace openbook;
using namespace filesystem;
using namespace messages;


Connection::Connection(QString hostname, int port, QObject *parent) :
    QObject(parent)
{
    socket = new QTcpSocket();

    socket->connectToHost(hostname,port);

    setAuthReq();


    connect(socket,SIGNAL(readyRead()),this,SLOT(read_data()));

}
void Connection::setDisplayName( const QString& name )
{


    SetDisplayName nameset;
    nameset.set_displayname(name.toAscii());

    std::ostringstream out;
    nameset.SerializeToOstream(&out);
    QByteArray byteArray(out.str().c_str());


    #pragma pack(1)
    struct header{
        int16_t size;
        int8_t id;
    };
    #pragma pack()

    header *h = new header;
    h->size = byteArray.size()+1;
    h->id = MSG_SET_DISPLAY_NAME;



    char *p = (char*)h;
    QByteArray b(p,3);
    qDebug()<<b.length();
    byteArray.prepend(b);
    qDebug()<<byteArray.toHex();

    socket->write(byteArray);


    SetDisplayName read;
    qDebug()<<read.ParseFromArray(byteArray.right(byteArray.length()-3).data(),byteArray.length()-3);

    if(socket->waitForBytesWritten(3000))
    {
        qDebug()<<"Written";
    }else{
        qDebug()<<"Failed to write";
    }
    if(socket->waitForReadyRead(3000))
    {
        QByteArray response = socket->readAll().toHex();


    }else{
        qDebug()<<"FAIL TO READ";

    }

}

QString Connection::getMountPoints()
{

    std::ostringstream out;


    messages::GetBackendInfo* msg =
                new messages::GetBackendInfo();
    // fill the message
    msg->set_req(messages::KNOWN_PEERS);

    msg->SerializeToOstream(&out);
    QByteArray byteArray(out.str().c_str());

    #pragma pack(1)
    struct header{
        int16_t size;
        int8_t id;
    };
    #pragma pack()

    header *h = new header;
    h->size = byteArray.size()+1;
    h->id = MSG_GET_BACKEND_INFO;



    char *p = (char*)h;
    QByteArray b(p,3);
    qDebug()<<b.length();
    byteArray.prepend(b);
    qDebug()<<byteArray.toHex();

    socket->write(byteArray);

    socket->waitForBytesWritten();



    return "Success!";

}

void Connection::read_data()
{
    QByteArray response;
    response = socket->readAll();

    qDebug()<<"Response data:";
    qDebug()<<response.data();

    qDebug()<<response.toHex();

}

void Connection::setAuthReq()
{
    AuthRequest *authReq = new AuthRequest();
    authReq->set_display_name("CLUI");
    authReq->set_public_key("UserInterface");


    std::ostringstream out;
    authReq->SerializeToOstream(&out);
    QByteArray byteArray(out.str().c_str());


    #pragma pack(1)
    struct header{
        int16_t size;
        int8_t id;
    };
    #pragma pack()

    header *h = new header;
    h->size = byteArray.size()+1;
    h->id = MSG_AUTH_REQ;


    char *p = (char*)h;
    QByteArray b(p,3);
    qDebug()<<b.length();
    byteArray.prepend(b);
    qDebug()<<byteArray.toHex();

    socket->write(byteArray);

    if(socket->waitForBytesWritten(3000))
    {
        qDebug()<<"Written";
    }else{
        qDebug()<<"Failed to write";
    }
    if(socket->waitForReadyRead(3000))
    {
        qDebug()<<socket->readAll().toHex();
    }else{
        qDebug()<<"FAIL TO READ";

    }


}

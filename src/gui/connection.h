#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <arpa/inet.h>
#include <sstream>
#include "messages.pb.h"


using namespace openbook;
using namespace filesystem;
using namespace messages;


class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QString hostname, int port, QObject *parent = 0);
    
    void setAuthReq();
    void setDisplayName( const QString& name );
    QString getMountPoints();

signals:
    
public slots:
    void read_data();


private:

    QTcpSocket *socket;

    enum MessageId
    {
        MSG_QUIT,
        MSG_PING,
        MSG_PONG,
        MSG_SET_DISPLAY_NAME,
        MSG_SET_DATA_DIR,
        MSG_SET_LOCAL_SOCKET,
        MSG_SET_REMOTE_SOCKET,
        MSG_SET_CLIENT_SOCKET,
        MSG_SET_MAX_CONN,
        MSG_LOAD_CONFIG,
        MSG_SAVE_CONFIG,
        MSG_ATTEMPT_CONNECT,
        MSG_ADD_MOUNT_POINT,
        MSG_REMOVE_MOUNT_POINT,
        MSG_UI_REPLY,
        MSG_GET_BACKEND_INFO,
        MSG_PEER_LIST,
        MSG_MOUNT_LIST,
        MSG_START_SYNC,
        MSG_LEADER_ELECT,
        MSG_DH_PARAMS,
        MSG_KEY_EXCHANGE,
        MSG_CEK,
        MSG_AUTH_REQ,
        MSG_AUTH_CHALLENGE,
        MSG_AUTH_SOLN,
        MSG_AUTH_RESULT,
        MSG_SUBSCRIBE,
        MSG_UNSUBSCRIBE,
        MSG_ID_MAP,
        MSG_NODE_INFO,
        MSG_NEW_VERSION,
        MSG_REQUEST_FILE,
        MSG_FILE_CHUNK,
        MSG_DIR_CHUNK,
        MSG_INVALID,
        NUM_MSG = MSG_INVALID,
    };

};

#endif // CONNECTION_H

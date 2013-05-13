#include "connection.h"
#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "SetDataDir.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

SetDataDir::SetDataDir(QString port):
    Options(port)
    {}

void SetDataDir::go(QString data_dir){
    FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        Marshall marshall;        //< create a marshaller
        marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        handshake(marshall);      //< perform handshake protocol

        // send the message
        messages::SetDataDir* msg =
            new messages::SetDataDir();
        // fill the message
        msg->set_datadir(data_dir.toUtf8().constData());

        // send the message to the backend
        marshall.writeMsg(msg);

        // wait for the reply
        RefPtr<AutoMessage> reply = marshall.read();

        // if the backend replied with a message we weren't expecting then
        // print an error
        if( reply->type != MSG_UI_REPLY )
        {
                std::cerr << "Unexpected reply of type: "
                  << messageIdToString( reply->type )
                  << "\n";
        }
        // otherwise print the result of the operation
        else
        {
        messages::UserInterfaceReply* msg =
                static_cast<messages::UserInterfaceReply*>(reply->msg);
           std::cout << "Server reply: "
                  << "\n    ok? : " << (msg->ok() ? "YES" : "NO")
                  << "\nmessage : " << msg->msg()
                  << "\n";
        }
}
const std::string SetDataDir::COMMAND       = "mounts";
const std::string SetDataDir::DESCRIPTION   = "all current mount points";

}
}
}

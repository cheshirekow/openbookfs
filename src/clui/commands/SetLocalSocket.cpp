
#include "connection.h"
#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "SetLocalSocket.h"

namespace   openbook {
namespace filesystem {
namespace       clui {


const std::string SetLocalSocket::COMMAND
    = "localSocket";

const std::string SetLocalSocket::DESCRIPTION
    = "the socket to listen on for UI connections";

SetLocalSocket::SetLocalSocket(TCLAP::CmdLine& cmd):
    Options(cmd),
    port(
        "local socket",
        "port number for socket",
        true,
        0,
        "local socket",
        cmd)
{}
	

void SetLocalSocket::go()
{
    FdPtr_t sockfd = connectToClient(*this);    //< create a connection
    Marshall marshall;        //< create a marshaller
    marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
    handshake(marshall);      //< perform handshake protocol

    // send the message
    messages::SetLocalSocket* msg =
        new messages::SetLocalSocket();
    // fill the message
    msg->set_port(port.getValue());

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



}
}
}


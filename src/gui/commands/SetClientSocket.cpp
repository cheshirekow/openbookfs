
#include "connection.h"
#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "SetClientSocket.h"

namespace   openbook {
namespace filesystem {
namespace       gui {


const std::string SetClientSocket::COMMAND      = "clientSocket";
const std::string SetClientSocket::DESCRIPTION  = "the socket to listen on for peers";


SetClientSocket::SetClientSocket(TCLAP::CmdLine& cmd):
    Options(),
    addressFamily(
        "address family",
        "AF_INET, AF_INET6, AF_UNIX",
        true,
        0,
        "address family",
        cmd),
    node( "address",
                "remote network interface to use",
                true,
                "any",
                "host",
                cmd
                )
    {}
	

void SetClientSocket::go(){
    FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        Marshall marshall;        //< create a marshaller
        marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        handshake(marshall);      //< perform handshake protocol

        // send the message
        messages::SetClientSocket* msg =
            new messages::SetClientSocket();
        // fill the message
        msg->set_addressfamily(addressFamily.getValue());
    msg->set_node(node.getValue());

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


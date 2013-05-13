#include "connection.h"
#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "SetRemoteSocket.h"

namespace   openbook {
namespace filesystem {
namespace       gui {


	const std::string SetRemoteSocket::COMMAND = "set";
    const std::string SetRemoteSocket::DESCRIPTION = "set remote socket";
	
    SetRemoteSocket::SetRemoteSocket():
    Options(),
	addressFamily(
		"address family",
		"AF_INET, AF_INET6, AF_UNIX",
		true,
		0,
        "address family"),
	node( "address",
                "remote network interface to use",
                true,
                "any",
                "host"
                ),
	remoteService(	"port",
                "remote port number / service name to use",
                true,
                "3031",
                "port"
		)
	
		
	{}
	

void SetRemoteSocket::go(){
		FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        	Marshall marshall;        //< create a marshaller
       		marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        	handshake(marshall);      //< perform handshake protocol

        	// send the message
        	messages::SetRemoteSocket* msg =
                new messages::SetRemoteSocket();
        	// fill the message
        	msg->set_addressfamily(addressFamily.getValue());
		msg->set_node(node.getValue());
		msg->set_service(remoteService.getValue());		

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

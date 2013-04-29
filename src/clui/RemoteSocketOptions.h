#ifndef OPENBOOK_FS_CLUI_REMOTESOCKETOPTIONS_H_
#define OPENBOOK_FS_CLUI_REMOTESOCKETOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class RemoteSocketOptions:
	public Options
{
	TCLAP::UnlabeledValueArg<int> addressFamily;
	TCLAP::UnlabeledValueArg<std::string> node;
	TCLAP::UnlabeledValueArg<std::string> remoteService;
	
	public:
	RemoteSocketOptions(TCLAP::CmdLine& cmd):
	Options(cmd),
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
                ),
	remoteService(	"port",
                "remote port number / service name to use",
                true,
                "3031",
                "port",
                cmd
		)
	
		
	{}
	

	void go(){
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
};

}
}
}

#endif

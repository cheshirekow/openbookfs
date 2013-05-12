#ifndef OPENBOOK_FS_GUI_MAXCONNECTIONSOPTIONS_H_
#define OPENBOOK_FS_GUI_MAXCONNECTIONSOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

class MaxConnectionsOptions:
	public Options
{
	TCLAP::UnlabeledValueArg<int> maxConn;
	
	public:
	MaxConnectionsOptions(TCLAP::CmdLine& cmd):
	Options(),
	maxConn(
		"max connections",
		"maximum number of connections",
		true,
		10,
		"max connections",
		cmd)
		
	{}
	

	void go(){
		FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        	Marshall marshall;        //< create a marshaller
       		marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        	handshake(marshall);      //< perform handshake protocol

        	// send the message
        	messages::SetMaxConnections* msg =
                new messages::SetMaxConnections();
        	// fill the message
        	msg->set_maxconn(maxConn.getValue());

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

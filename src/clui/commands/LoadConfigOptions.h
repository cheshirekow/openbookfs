#ifndef OPENBOOK_FS_CLUI_LOADCONFIGOPTIONS_H_
#define OPENBOOK_FS_CLUI_LOADCONFIGOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class LoadConfigOptions:
	public Options
{
	TCLAP::UnlabeledValueArg<std::string> fileName;
	
	public:
	LoadConfigOptions(TCLAP::CmdLine& cmd):
	Options(cmd),
	fileName(
		"config file name",
		"config file to load",
		true,
		"",
		"load file",
		cmd)
	{}
	

	void go(){
		FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        	Marshall marshall;        //< create a marshaller
       		marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        	handshake(marshall);      //< perform handshake protocol

        	// send the message
        	messages::LoadConfig* msg =
                new messages::LoadConfig();
        	// fill the message
        	msg->set_filename(fileName.getValue());

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

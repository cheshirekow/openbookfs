#ifndef OPENBOOK_FS_GUI_SETDATADIR_H_
#define OPENBOOK_FS_GUI_SETDATADIR_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

class SetDataDir:
	public Options
{

	TCLAP::UnlabeledValueArg<std::string> dataDir;
	
	public:
	static const std::string COMMAND;
	static const std::string DESCRIPTION;

	SetDataDir(TCLAP::CmdLine& cmd):
	Options(),
	dataDir(
		"data directory",
		"data storage location",
		true,
		"",
		"data directory",
		cmd)
	{}
	

	void go(){
		FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        	Marshall marshall;        //< create a marshaller
       		marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        	handshake(marshall);      //< perform handshake protocol

        	// send the message
        	messages::SetDataDir* msg =
                new messages::SetDataDir();
        	// fill the message
        	msg->set_datadir(dataDir.getValue());

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


const std::string SetDataDir::COMMAND       = "dataDir";
const std::string SetDataDir::DESCRIPTION   = "where data is stored";

}
}
}

#endif

#ifndef OPENBOOK_FS_CLUI_LISTKNOWNCLIENTOPTIONS_H_
#define OPENBOOK_FS_CLUI_LISTKNOWNCLIENTOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class ListKnownClientOptions:
	public Options
{


	public:
		ListKnownClientOptions(TCLAP::CmdLine& cmd):
			Options(cmd)

			{}


				

	void go(){
	FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        Marshall marshall;        //< create a marshaller
        marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        handshake(marshall);      //< perform handshake protocol

        // send the message
        messages::ListKnownClient* msg =
                new messages::ListKnownClient();
        // fill the message
        //msg->set_imp(imp.getValue());

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

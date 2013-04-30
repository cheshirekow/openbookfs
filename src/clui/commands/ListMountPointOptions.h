#ifndef OPENBOOK_FS_CLUI_LISTMOUNTPOINTOPTIONS_H_
#define OPENBOOK_FS_CLUI_LISTMOUNTPOINTOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class ListMountPointOptions:
	public Options
{
	public:
        static const std::string COMMAND;
		ListMountPointOptions(TCLAP::CmdLine& cmd):
			Options(cmd)
			{}

	void go(){
		FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        Marshall marshall;        //< create a marshaller
        marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
	std::cout << "hhh" << std::endl;        
	handshake(marshall);      //< perform handshake protocol

        // send the message
        messages::GetBackendInfo* msg =
                new messages::GetBackendInfo();
        // fill the message
        msg->set_req(messages::MOUNT_POINTS);

        // fill the message
        //msg->set_imp(imp.getValue());
        // send the message to the backend
        marshall.writeMsg(msg);

        // wait for the reply
        RefPtr<AutoMessage> reply = marshall.read();

        // if the backend replied with a message we weren't expecting then
        // print an error
        if( reply->type != MSG_MOUNT_LIST )
        {
            std::cerr << "Unexpected reply of type: "
                      << messageIdToString( reply->type )
                      << "\n";
        }
        // otherwise print the result of the operation
        else
        {
            messages::MountList* msg =
                    static_cast<messages::MountList*>(reply->msg);

            std::size_t lenRelDir   = 0;
            std::size_t lenPath     = 0;

            // compute field lengths
            for(int i=0; i < msg->mounts_size(); i++)
            {
                lenRelDir = std::max(lenRelDir,
                                        msg->mounts(i).relpath().length() );
                lenPath   = std::max(lenPath,
                                        msg->mounts(i).path().length() );
            }

            std::string relDirHeader    = "relDir";
            std::string pathHeader      = "mountPoint";
            std::string argsHeader      = "args";

            // print headers
            if( lenRelDir < relDirHeader.size() )
                lenRelDir = relDirHeader.size();

            if( lenPath < pathHeader.size() )
                lenPath = pathHeader.size();


            std::cout << relDirHeader;
            for(int i=0; i < 3 + lenRelDir - relDirHeader.size(); i++)
                std::cout << " ";

            std::cout << pathHeader;
            for(int i=0; i < 4 + lenPath - pathHeader.size(); i++)
                std::cout << " ";

            std::cout << argsHeader;
            std::cout <<"\n";

            // now print out data
            for(int i=0; i < msg->mounts_size(); i++)
            {
                int nSpaces = 0;

                std::string out = msg->mounts(i).relpath();
                nSpaces = lenRelDir - out.length();
                std::cout << out;
                for(int i=0; i < nSpaces; i++)
                    std::cout << " ";
                std::cout << "   ";

                out = msg->mounts(i).path();
                nSpaces = lenPath - out.length();
                std::cout << out;
                for(int i=0; i < nSpaces; i++)
                    std::cout << " ";
                std::cout << "   ";

                for(int j=0; j < msg->mounts(i).argv_size(); j++)
                    std::cout << msg->mounts(i).argv(j) << " ";
                std::cout << "\n";
            }
        }
	}

};


const std::string ListMountPointOptions::COMMAND = "mounts";

}
}
}

#endif

#ifndef OPENBOOK_FS_CLUI_LISTKNOWNCLIENTOPTIONS_H_
#define OPENBOOK_FS_CLUI_LISTKNOWNCLIENTOPTIONS_H_

#include "Options.h"
#include <crypto++/sha.h>
#include <cmath>

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
        messages::GetBackendInfo* msg =
                new messages::GetBackendInfo();
        // fill the message
        msg->set_req(messages::KNOWN_PEERS);

        // send the message to the backend
        marshall.writeMsg(msg);

        // wait for the reply
        RefPtr<AutoMessage> reply = marshall.read();

        // if the backend replied with a message we weren't expecting then
        // print an error
        if( reply->type != MSG_PEER_LIST )
        {
            std::cerr << "Unexpected reply of type: "
                      << messageIdToString( reply->type )
                      << "\n";
        }
        // otherwise print the result of the operation
        else
        {
            messages::PeerList* msg =
                    static_cast<messages::PeerList*>(reply->msg);


            std::size_t lenId   = 0;
            std::size_t lenName = 0;
            std::size_t lenKey  = CryptoPP::SHA256::DIGESTSIZE*3/8;

            // compute field lengths
            for(int i=0; i < msg->peers_size(); i++)
            {
                std::stringstream strm;
                strm << msg->peers(i).peerid();
                lenId   = std::max(lenId,   strm.str().length() );
                lenName = std::max(lenName, msg->peers(i).displayname().length() );
            }

            // now print out data
            for(int i=0; i < msg->peers_size(); i++)
            {
                int nSpaces = 0;

                std::stringstream strm;
                strm << msg->peers(i).peerid();
                nSpaces = lenId - strm.str().length();
                for(int i=0; i < nSpaces; i++)
                    strm << " ";
                std::cout << strm.str() << "   ";

                strm.str("");
                strm << msg->peers(i).displayname();
                nSpaces = lenName - strm.str().length();
                for(int i=0; i < nSpaces; i++)
                    strm << " ";
                std::cout << strm.str() << "   ";

                using namespace CryptoPP;
                typedef SHA1 sha;
                std::string pubkey = msg->peers(i).publickey();
                byte digest[sha::DIGESTSIZE];
                sha().CalculateDigest(digest,(byte*)&pubkey[0],pubkey.length());

                std::cout << std::hex;
                for(int i=0; i < sha::DIGESTSIZE; i++)
                {
                    std::cout << std::setfill('0')
                              << std::setw(2)
                              << (int)digest[i];
                    if( i < sha::DIGESTSIZE-1 )
                        std::cout << ":";
                }
                std::cout << "\n";
            }

        }
	}

};

}
}
}

#endif

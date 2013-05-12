/*
 *  Copyright (C) 2012 Josh Bialkowski (jbialk@mit.edu)
 *
 *  This file is part of openbook.
 *
 *  openbook is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openbook is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openbook.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 *  @file   /home/josh/Codes/cpp/openbookfs/src/gui/commands/Connect.cpp
 *
 *  @date   Apr 30, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "connection.h"
#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "Connect.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

const std::string Connect::COMMAND      = "connect";
const std::string Connect::DESCRIPTION  = "initiate a connection with a peer";

Connect::Connect():
    Options()/*,
    isLocal("l",    //< short flag character, usage: "-l"
        "local",    //< long flag, usage: "--local"
        // help message
        "indicates that the remote interface is a local connection"
            ", i.e localhost, 127.0.0.1, etc",
        cmd         //< parser to add this argument to
        ),
    remoteNode(
        "address",  //< unique name, not really used anywhere
        // help message
        "remote network interface to use",
        true,       //< required?
        "any",      //< default value
        "host",     //< placeholder for this argument in the help
        cmd
        ),
    remoteService(
        "port",
        "remote port number / service name to use",
        true,
        "3031",
        "port",
        cmd)*/
{}

void Connect::go()
{
    FdPtr_t sockfd = connectToClient(*this);
    Marshall marshall;
    marshall.setFd(*sockfd);
    handshake(marshall);

    // send the message
    messages::AttemptConnection* msg =
            new messages::AttemptConnection();
    msg->set_isremote(true);
    msg->set_node("localhost");
    msg->set_service("3033");
    marshall.writeMsg(msg);

    // wait for the reply
    RefPtr<AutoMessage> reply = marshall.read();

    // print the result
    if( reply->type != MSG_UI_REPLY )
    {
        std::cerr << "Unexpected reply of type: "
                  << messageIdToString( reply->type )
                  << "\n";
    }
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






} //< namespace gui
} //< namespace filesystem
} //< namespace openbook


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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/gui/commands/StartSync.cpp
 *
 *  @date   May 5, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */




#include "connection.h"
#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "StartSync.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

const std::string StartSync::COMMAND      = "sync";
const std::string StartSync::DESCRIPTION  = "begin a full sync with a peer";

StartSync::StartSync():
    Options(),
    peerId(
        "peerId",   //< unique name, not really used anywhere
        // help message
        "peer to sync with ",
        true,       //< required?
        0,          //< default value
        "peerId"   //< placeholder for this argument in the help
        )
{}

void StartSync::go()
{
    FdPtr_t sockfd = connectToClient(*this);
    Marshall marshall;
    marshall.setFd(*sockfd);
    handshake(marshall);

    // send the message
    messages::StartSync* msg =
            new messages::StartSync();
    msg->set_peerid(peerId.getValue());
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

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
 *  @file   src/gui/SetDisplayName.cpp
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "connection.h"
#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "SetDisplayName.h"


namespace   openbook {
namespace filesystem {
namespace       gui {

SetDisplayName::SetDisplayName():
    Options()
{}

void SetDisplayName::go()
{
    FdPtr_t sockfd = connectToClient(*this);
    Marshall marshall;
    marshall.setFd(*sockfd);
    handshake(marshall);



    std::cout<<"Here Final"<<std::endl;
    // send the message
    messages::SetDisplayName* msg = new messages::SetDisplayName();
    char *value = "Displayname";

    msg->set_displayname("value");
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


const std::string SetDisplayName::COMMAND       = "displayName";
const std::string SetDisplayName::DESCRIPTION   = "human readable machine name";


} //< namespace gui
} //< namespace filesystem
} //< namespace openbook





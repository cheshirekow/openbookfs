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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/clui/DisplayNameOptions.h
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_CLUI_DISPLAYNAMEOPTIONS_H_
#define OPENBOOK_FS_CLUI_DISPLAYNAMEOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class DisplayNameOptions:
    public Options
{
    TCLAP::UnlabeledValueArg<std::string> displayName;    ///< display name

    public:
        DisplayNameOptions( TCLAP::CmdLine& cmd ):
            Options(cmd),
            displayName(
                "displayName",
                "remote network interface to use",
                true,
                "test_x",
                "displayName",
                cmd
                )
        {}

        void go()
        {
            FdPtr_t sockfd = connectToClient(*this);
            Marshall marshall;
            marshall.setFd(*sockfd);
            handshake(marshall);

            // send the message
            messages::SetDisplayName* msg = new messages::SetDisplayName();
            msg->set_displayname(   displayName.getValue() );
            marshall.writeMsg(msg);

            // wait for the reply
            RefPtr<AutoMessage> reply = marshall.read();

            // print the result
            if( reply->type != MSG_SET_DISPLAY_NAME )
            {
                std::cerr << "Unexpected reply of type: "
                          << messageIdToString( reply->type )
                          << "\n";
            }
            else
                std::cout << "Ok!\n";
        }
};



} //< namespace clui
} //< namespace filesystem
} //< namespace openbook





#endif // DISPLAYNAMEOPTIONS_H_

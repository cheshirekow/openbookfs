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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/clui/Options.h
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_CLUI_OPTIONS_H_
#define OPENBOOK_FS_CLUI_OPTIONS_H_

#include <sys/socket.h>
#include <string>
#include <tclap/CmdLine.h>

#include "messages.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

/// common command line options
class Options
{
    TCLAP::ValueArg<std::string> clientFamily;  ///< client address family
    TCLAP::ValueArg<std::string> clientNode;    ///< client address
    TCLAP::ValueArg<std::string> clientService; ///< client port

    public:
        Options( TCLAP::CmdLine& cmd ):
            clientFamily(
                "a",
                "address-family",
                "address family of interface to use: AF_[INET|INET6|UNIX|UNSPEC]",
                false,
                "AF_INET",
                "ai_family",
                cmd
                ),
            clientNode(
                "i",
                "client-iface",
                "client network interface to use",
                false,
                "any",
                "iface",
                cmd
                ),
            clientService(
                "p",
                "client-port",
                "client port number / service name to use",
                false,
                "3031",
                "port",
                cmd)
        {}

        int get_clientFamily()
        {
            std::string clientFamilyStr = clientFamily.getValue();
            int family = AF_INET;
            if( clientFamilyStr == "AF_INET" )
                family = AF_INET;
            else if( clientFamilyStr == "AF_INET6" )
                family = AF_INET6;
            else if( clientFamilyStr == "AF_UNIX" )
                family = AF_UNIX;
            else if( clientFamilyStr == "AF_UNSPEC" )
                family = AF_UNSPEC;

            return family;
        }

        std::string get_clientNode()
        {
            std::string clientNodeStr = clientNode.getValue();
            if( clientNodeStr == "any" )
                return "";
            else
                return clientNodeStr;
        }

        std::string get_clientService()
        {
            return clientService.getValue();
        }
};


class CmdLine:
    public  TCLAP::CmdLine
{
    public:
        CmdLine(const std::string& cmd,
                const std::string& message,
                const char delimiter = ' ',
                const std::string& version = "none",
                bool helpAndVersion = true
                ):
            TCLAP::CmdLine(message,delimiter,version,helpAndVersion)
        {
            _progName = "obfs " + cmd;
        }
};

} //< namespace clui
} //< namespace filesystem
} //< namespace openbook















#endif // OPTIONS_H_

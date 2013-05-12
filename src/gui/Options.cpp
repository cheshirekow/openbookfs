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
 *  @file   src/gui/Options.cpp
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       gui {




Options::Options():
    clientFamily(
        "a",                //< short flag name, usage: "-a [value]"
        "address-family",   //< long flag: "--address-family [value]"
        // description, is printed in the help message
        "address family of interface to use: "
            "AF_[INET|INET6|UNIX|UNSPEC]",
        false,              //< required?
        "AF_INET",          //< default value
        "ai_family"        //< placeholder for this argument in help
        ),
    clientNode(
        "i",
        "client-iface",
        "client network interface to use",
        false,
        "localhost",
        "iface"),
    clientService(
        "p",
        "client-port",
        "client port number / service name to use",
        false,
        "3030",
        "port")
{}



int Options::get_clientFamily()
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

std::string Options::get_clientNode()
{
    std::string clientNodeStr = clientNode.getValue();
    if( clientNodeStr == "any" )
        return "";
    else
        return clientNodeStr;
}

std::string Options::get_clientService()
{
    return clientService.getValue();
}


CmdLine::CmdLine(const std::string& cmd,
        const std::string& message,
        const char delimiter,
        const std::string& version,
        bool helpAndVersion
        ):
    TCLAP::CmdLine(message,delimiter,version,helpAndVersion)
{
    _progName = "obfs " + cmd;
}




} //< namespace gui
} //< namespace filesystem
} //< namespace openbook


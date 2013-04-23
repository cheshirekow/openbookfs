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
 *  @file   src/clui/Options.h
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
/**
 *  Contains command line options for connectiong to the local backend.
 *
 *  Bruno: in order to add new commands you need to create a subclass of this
 *  class. It should have as member variables any TCLAP arguments which are
 *  required to perform that command. See the tclap documentation at
 *  http://tclap.sourceforge.net/manual.html for an overview, or the API
 *  documentation on that site for how to construct the different types.
 *
 *  The basic idea is that declare a TCLAP::xxxArg and in it's constructor
 *  pass all the information for parsing and documentation. See the commands
 *  for the clientFamily member below for how to declare an argument which
 *  has a value. See ConnectOptions::isLocal for an argument which doesn't have
 *  a value (i.e. a switch or flag), and see ConnectOptions::remoteNode for
 *  an argument which doesn't have a flag.
 *
 *  Here is a
 *  boiler-plate code for the go() method of derived classes. See
 *  ConnectOptions or DisplayNameOptions for a concrete example.
 *
 *@code
        FdPtr_t sockfd = connectToClient(*this);    //< create a connection
        Marshall marshall;        //< create a marshaller
        marshall.setFd(*sockfd);  //< tell the marshaller the socket to use
        handshake(marshall);      //< perform handshake protocol

        // send the message
        messages::SomeMessageType* msg =
                new messages::SomeMessageType();
        // fill the message
        msg->set_variableA(0);
        msg->set_variableB(0);
        // etc...

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
@endcode
 *
 *
 *
 */
class Options
{
    TCLAP::ValueArg<std::string> clientFamily;  ///< client address family
    TCLAP::ValueArg<std::string> clientNode;    ///< client address
    TCLAP::ValueArg<std::string> clientService; ///< client port

    public:
        Options( TCLAP::CmdLine& cmd ):
            clientFamily(
                "a",                //< short flag name, usage: "-a [value]"
                "address-family",   //< long flag: "--address-family [value]"
                // description, is printed in the help message
                "address family of interface to use: "
                    "AF_[INET|INET6|UNIX|UNSPEC]",
                false,              //< required?
                "AF_INET",          //< default value
                "ai_family",        //< placeholder for this argument in help
                cmd                 //< the TCLAP::CmdLine object to use
                ),
            clientNode(
                "i",
                "client-iface",
                "client network interface to use",
                false,
                "localhost",
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

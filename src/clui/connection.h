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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/clui/connection.h
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_CLUI_CONNECTION_H_
#define OPENBOOK_FS_CLUI_CONNECTION_H_

#include <cerrno>

#include "FileDescriptor.h"
#include "Marshall.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {


typedef RefPtr<FileDescriptor>  FdPtr_t;

/// create a connection to the desired server
FdPtr_t connectToClient( Options& opts )
{
    // defaults
    addrinfo  hints;
    addrinfo* found;
    memset(&hints,0,sizeof(addrinfo));
    hints.ai_family   = opts.get_clientFamily();
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    // create a socket
    const char* node    = 0;
    const char* service = 0;

    int sockfd = socket(hints.ai_family,
                        hints.ai_socktype ,
                        hints.ai_protocol);
    if(sockfd < 0)
    {
        std::cerr << "Failed create client socket\n";
        return FdPtr_t();
    }

    FdPtr_t clientfd = FileDescriptor::create(sockfd);

    // attempt to make connection
    std::string clientNode    = opts.get_clientNode();
    std::string clientService = opts.get_clientService();
    node    = clientNode.c_str();
    service = clientService.c_str();

    std::cout << "Searching for host addr matching "
              << node << ":" << service << std::endl;

    int result = getaddrinfo(node,service,&hints,&found);
    if( result < 0 )
    {
        ex()() << "Failed to find an interface which matches: "
               << node << ":" << service
               << "\nErrno is " << errno << " : " << strerror(errno);
    }

    addrinfo* addr = 0;
    for( addr = found; addr; addr = addr->ai_next )
    {
        std::cout << "Attempting to connect to server:"
                  << "\n   family: " << addr->ai_family
                  << "\n     type: " << addr->ai_socktype
                  << "\n protocol: " << addr->ai_protocol
                  << std::endl;

        int connectResult =
                connect( *clientfd, addr->ai_addr, addr->ai_addrlen );
        if (connectResult < 0 )
        {
            std::cerr << "Connection failed, errno " << errno << " : "
                      << strerror(errno) << "\n";
            continue;
        }
        else
            break;
    }

    freeaddrinfo(found);

    if( !addr )
        ex()() << "None of the matched server interfaces work";

    return clientfd;
}



static void validate_message( RefPtr<AutoMessage> msg, MessageId expected )
{
    if( msg->type != expected )
    {
        ex()() << "Protocol Error: expected "
               << messageIdToString(expected)
               << " from peer, instead got"
               << messageIdToString(msg->type)
               << "(" << (int)msg->type << ")";
    }
}


/// perform handshake to notify client that we are a ui
void handshake( Marshall& marshall )
{
    namespace msgs = messages;
    using namespace pthreads;
    using namespace CryptoPP;

    // trade public keys
    msgs::AuthRequest* authReq = new msgs::AuthRequest();
    authReq->set_display_name("CLUI");
    authReq->set_public_key("clui");
    marshall.writeMsg( authReq );

    RefPtr<AutoMessage> recv = marshall.read( );
    validate_message( recv, MSG_AUTH_REQ );
    authReq = static_cast<msgs::AuthRequest*>( recv->msg );

    std::cout << "Handshake with client:"
              << "\n" << authReq->display_name()
              << "\n" << authReq->public_key()
              << "\n";
}


} //< namespace clui
} //< namespace filesystem
} //< namespace openbook



#endif // CONNECTION_H_

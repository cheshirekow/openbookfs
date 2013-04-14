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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/SocketListener.cpp
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */



#include <cerrno>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <sys/socket.h>

#include "global.h"
#include "ExceptionStream.h"
#include "SelectSpec.h"
#include "SocketListener.h"

namespace   openbook {
namespace filesystem {

SocketListener::SocketListener()
{
    m_mutex.init();
    m_sockfd = 0;
    memset(&m_hints, 0, sizeof(addrinfo));
    m_hints.ai_family   = AF_UNSPEC;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_flags    = AI_PASSIVE | AI_NUMERICSERV;
    m_node   = "localhost";
    m_service= "";
}

SocketListener::~SocketListener()
{
    m_mutex.destroy();
}

void SocketListener::setInterface(const std::string& node, int port )
{
    pthreads::ScopedLock( m_mutex );
    m_node    = node;

    std::stringstream strm;
    strm << port;
    m_service = strm.str();

    m_notify.notify();
}

void SocketListener::setInterface(int family,
                                    const std::string& node,
                                    const std::string& service )
{
    pthreads::ScopedLock( m_mutex );
    m_hints.ai_family = family;
    m_node    = node;
    m_service = service;
    m_notify.notify();
}

void SocketListener::mainLoop()
{
    bool shouldQuit = false;
    while(!shouldQuit)
    {
        m_sockfd        = 0;
        addrinfo* found = 0; // matching interfaces
        addrinfo* addr  = 0; // decided interface

        try
        {
            // need to lock hints while we search
            pthreads::ScopedLock(m_mutex);
            m_notify.clear();

            const char* node    = m_node.length()    > 0 ? m_node.c_str()    : 0;
            const char* service = m_service.length() > 0 ? m_service.c_str() : 0;
            int result =
                getaddrinfo(node,service,&m_hints,&found);

            if( result < 0 )
            {
                ex()() << "Failed to find an matching interface for\n "
                       << "\n   family: " << m_hints.ai_family
                       << "\n     node: " << m_node
                       << "\n  service: " << m_service
                       << "\n errno is: " << errno << " : " << strerror(errno);
            }

            addrinfo* addr = found;
            for( ; addr; addr = addr->ai_next )
            {
                std::stringstream msg;
                msg << "Attempting to create socket:"
                          << "\n   family: " << addr->ai_family
                          << "\n     type: " << addr->ai_socktype
                          << "\n protocol: " << addr->ai_protocol
                          << "\n";
                std::cout << msg.str();

                m_sockfd = socket(addr->ai_family,
                                    addr->ai_socktype | O_NONBLOCK,
                                    addr->ai_protocol);

                char host[NI_MAXHOST];
                char port[NI_MAXSERV];
                memset(host, 0, sizeof(host));
                memset(port, 0, sizeof(port));
                getnameinfo( (sockaddr*)addr->ai_addr, addr->ai_addrlen,
                             host, sizeof(host),
                             port, sizeof(port),
                             NI_NUMERICHOST | NI_NUMERICSERV );

                msg.str("");
                msg << "Attempting to bind listener to "
                    << host << ":" << port << std::endl;
                std::cout << msg.str();

                if (m_sockfd < 0)
                    continue;
                else
                    break;
            }

            if( !addr )
                ex()() << "None of the matching interfaces work";
            std::cout << "Listener bound\n";

            // So that we can re-bind to it without TIME_WAIT problems
            int reuse_addr = 1;
            if( setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,
                                    &reuse_addr, sizeof(reuse_addr)) )
            {
                ex()() << "Failed to set SO_REUSEADDR on socket";
            }

            //  Bind the server socket
            if ( bind(m_sockfd, addr->ai_addr, addr->ai_addrlen) < 0)
                ex()() << "Failed to bind the server socket\n";



            //  Listen on the server socket, 10 max pending connections
            if (listen(m_sockfd, 10) < 0)
                ex()() << "Failed to listen on server socket\n";
        }
        catch( std::exception& ex )
        {
            std::cerr << "Error while setting up the socket: "
                      << ex.what() << std::endl;
            if( m_sockfd > 0 )
                close(m_sockfd);
            m_sockfd = 0;
        }

        if(found)
            freeaddrinfo(found);

        // for waiting until things happen
        SelectSpec selectMe;
        {
            using namespace select_spec;
            selectMe.gen()
                    ( g_termNote->readFd(), READ )  //< program exit
                    ( m_notify.readFd(), READ )     //< socket change
                    ( TimeVal(10,0) );              //< timeout
            if( m_sockfd )
                selectMe.add( m_sockfd, READ );     //< client connect
        }

        // receive loop
        bool shouldRestart = false;
        while(!shouldRestart && !shouldQuit)
        {
            using namespace select_spec;
            // if timeout
            if( !selectMe.wait() )
            {
                std::cout << "SocketListener timeout\n";
                continue;
            }
            // if we need to change our listen socket
            else if( selectMe.ready( m_notify.readFd(), READ) )
            {
                std::cout << "SocketListener notified, restarting\n";
                shouldRestart = true;
                break;
            }
            // if we need to quit
            else if( selectMe.ready( g_termNote->readFd(), READ ) )
            {
                std::cout << "SocketListener notified, quitting\n";
                shouldQuit = true;
                break;
            }

            std::cout << "SocketListner woke up to accept a connection"
                      << std::endl;
            sockaddr_storage clientaddr;
            unsigned int     addrlen  = sizeof(sockaddr_storage);
            char clienthost[NI_MAXHOST];
            char clientport[NI_MAXSERV];

            //  Wait for client connection (should not block)
            int clientsock = accept4(
                    m_sockfd,
                    (sockaddr*)&clientaddr,
                    &addrlen,
                    SOCK_NONBLOCK);

            if( clientsock < 0 )
            {
                switch( errno )
                {
                    case EWOULDBLOCK:
                    {
                        std::cout << "SocketListener: No pending connections"
                                  << std::endl;
                        continue;
                    }

                    default:
                    {
                        ex()() << "Failed to accept client connection: errno "
                               << errno << " : " << strerror(errno) ;
                        continue;
                    }
                }
            }


            memset(clienthost, 0, sizeof(clienthost));
            memset(clientport, 0, sizeof(clientport));
            getnameinfo( (sockaddr*)&clientaddr, addrlen,
                         clienthost, sizeof(clienthost),
                         clientport, sizeof(clientport),
                         NI_NUMERICHOST | NI_NUMERICSERV );

            std::stringstream msg;
            msg << "Client connected host=[" << clienthost
                << "] port=[" << clientport << "]" << "\n";
            std::cout << msg.str();

            sig_client(clientsock);
        }
    }
}

void* SocketListener::start( void* pObj )
{
    static_cast<SocketListener*>(pObj)->mainLoop();
    return pObj;
}

} // namespace filesystem
} // namespace openbook










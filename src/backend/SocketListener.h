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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/SocketListener.h
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_SOCKETLISTENER_H_
#define OPENBOOK_FS_SOCKETLISTENER_H_

#include <string>

#include <netdb.h>

#include <cpp-pthreads.h>
#include <sigc++/sigc++.h>

#include "NotifyPipe.h"
#include "FileDescriptor.h"


namespace   openbook {
namespace filesystem {

/// opens a server socket and listens for incoming client connections
/**
 *  The listener runs in it's own thread. The interface it listens on can
 *  be changed while it's running. Any accepted client connections are passed
 *  through sig_client
 *
 *  todo: make socket file descriptors a reference counted resource so that
 *        failures dont have to worry about cleaning them up
 */
class SocketListener
{
    public:
        typedef RefPtr<FileDescriptor>  FdPtr_t;

    private:
        NotifyPipe          m_notify;   ///< signals a restart
        pthreads::Mutex     m_mutex;    ///< locks data
        addrinfo            m_hints;    ///< interface to listen on
        std::string         m_node;     ///< interface to listen on
        std::string         m_service;  ///< interface to listen on
        int                 m_sockfd;   ///< listening socket fd

    public:
        /// when a peer connects, the socket fd is sent over this signal
        sigc::signal<void,FdPtr_t>  sig_client;

        /// initializes the mutex
        SocketListener();

        /// destroys the mutex
        ~SocketListener();

        /// changes the interface the listener should listen on and signals
        /// the listener to restart
        void setInterface( const std::string& node, int port );

        /// changes the interface the listener should listen on and signals
        /// the listener to restart
        void setInterface( int family,
                            const std::string& node,
                            const std::string& service);

        std::string getFamily()
        {
            switch( m_hints.ai_family)
            {
                case AF_INET:
                    return "AF_INET";
                case AF_INET6:
                    return "AF_INET6";
                default:
                    return "AF_UNSPEC";
            }
        }
        const std::string& getNode(){ return m_node; }
        const std::string& getService(){ return m_service; }

    private:
        /// opens a socket based on hints, listens for clients, and sends
        /// connections over sig_connect
        void mainLoop();

    public:
        /// static method for a pthread, pointer must be a pointer to a
        /// SocketListener, simply calls mainLoop()
        static void* start( void* pObj );

};

} // namespace filesystem
} // namespace openbook
















#endif // SOCKETLISTENER_H_

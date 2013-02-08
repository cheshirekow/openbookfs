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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/RequestHandler.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_HANDLER_H_
#define OPENBOOK_HANDLER_H_

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <cpp-pthreads.h>
#include <vector>
#include "Pool.h"

namespace   openbook {
namespace filesystem {




class RequestHandler
{
    private:
        typedef Pool<RequestHandler>    Pool_t;
        static const unsigned int sm_bufsize = 256;

        Pool_t*             m_pool;             ///< pool to which this belongs
        pthreads::Thread    m_thread;           ///< the thread we're running in
        pthreads::Mutex     m_mutex;            ///< locks this data
        char                m_buf[sm_bufsize];  ///< socket buffer
        int                 m_sock;             ///< socket fd

        void cleanup();

    public:
        RequestHandler();

        /// set the parent pointer
        void init( Pool_t* );

        /// start the handler interfacing with the client on the socket
        void start( int sockfd );

        /// makes this a callable object that can be sent to the thread
        void* operator()();

};



} // namespace filesystem
} // namespace openbook















#endif // HANDLER_H_

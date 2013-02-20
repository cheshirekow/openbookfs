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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/MessageHandler.h
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_MESSAGEHANDLER_H_
#define OPENBOOK_MESSAGEHANDLER_H_


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <cpp-pthreads.h>

#include "ExceptionStream.h"
#include "Pool.h"
#include "Queue.h"
#include "messages.h"
#include "ClientMessage.h"
#include "ClientMap.h"
#include "Server.h"

namespace   openbook {
namespace filesystem {
 namespace    server {


/// reads a message and does whatever the message says to do
class MessageHandler
{
    public:
        typedef Pool<MessageHandler>          Pool_t;
        typedef Queue<ClientMessage>          MsgQueue_t;

    private:
        Pool_t*             m_pool;             ///< pool to which this belongs
        MsgQueue_t*         m_msgQueue;         ///< global msg queue
        ClientMap*          m_clientMap;        ///< maps client ids to handlers
        pthreads::Thread    m_thread;           ///< the thread we're running in
        pthreads::Mutex     m_mutex;            ///< locks this data

        Server*             m_server;

        /// main method of the job handler, waits for jobs in the queue and
        /// then does them
        void* main();

        /// static method for pthreads, calls main()
        static void* dispatch_main( void* vp_h );

        /// typed message  handler
        void handleMessage( ClientMessage msg, messages::Ping*       upcast );
        void handleMessage( ClientMessage msg, messages::Pong*       upcast );
        void handleMessage( ClientMessage msg, messages::NewVersion* upcast );
        void handleMessage( ClientMessage msg, messages::FileChunk*  upcast );

    public:
        MessageHandler();
        ~MessageHandler();

        /// set the parent pointer and start DH parameter generation in
        /// detached thread
        void init( Pool_t*, MsgQueue_t*, Server*, ClientMap* );

        /// start the worker thread
        void start();

};



} // namespace server 
} // namespace filesystem
} // namespace openbook


#endif // MESSAGEHANDLER_H_

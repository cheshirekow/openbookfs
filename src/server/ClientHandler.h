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
 *  @file   src/server/ClientHandler.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_HANDLER_H_
#define OPENBOOK_HANDLER_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <set>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cpp-pthreads.h>
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/dh.h>
#include <crypto++/dh2.h>

#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "ExceptionStream.h"
#include "MessageBuffer.h"
#include "MessageHandler.h"
#include "Pool.h"
#include "Queue.h"
#include "Server.h"
#include "Synchronized.h"
#include "ClientMessage.h"
#include "ClientMap.h"


namespace   openbook {
namespace filesystem {
 namespace    server {

/// runtime error thrown by the client handler
class ClientException :
    public std::runtime_error
{
    public:
        ClientException( const std::string& msg ) throw():
            std::runtime_error(msg)
        {}

        virtual ~ClientException() throw(){}
};


/// dedicated RPC handler for a single client, manages both inbound and
/// outbound protocol buffer messages
class ClientHandler
{
    public:
        typedef ExceptionStream<ClientException> ex;
        typedef Pool<ClientHandler>              Pool_t;

        typedef Queue<ClientMessage>            InQueue_t;
        typedef Queue<TypedMessage>             OutQueue_t;

    private:
        static const unsigned int sm_bufsize = 256;

        unsigned int        m_clientId;         ///< incremented on reuse
        Pool_t*             m_pool;             ///< pool to which this belongs
        Server*             m_server;           ///< server configuration
        ClientMap*          m_clientMap;        ///< maps client ids to handlers
        pthreads::Thread    m_thread;           ///< the thread we're running in

        InQueue_t           m_inboundMessages;  ///< received message queue
        OutQueue_t          m_outboundMessages; ///< queue for messages to send
        MessageHandler      m_worker;           ///< worker for this client

        pthreads::Thread    m_listenThread;     ///< child thread for listening
        pthreads::Thread    m_shoutThread;      ///< child thread for shouting
        pthreads::Thread    m_workerThread;     ///< child thread for worker
        pthreads::Mutex     m_mutex;            ///< locks this data
        MessageBuffer       m_msg;              ///< message buffer
        int                 m_fd[2];            ///< socket and terminal fd

        // Diffie-Hellman Paramters
        CryptoPP::Integer   p,q,g;
        CryptoPP::SecByteBlock          m_cek;    ///< content encryption key
        CryptoPP::SecByteBlock          m_iv;     ///< initial vector
        CryptoPP::AutoSeededRandomPool  m_rng;    ///< random number gen

        /// initialize Diffie Hellman paramters (takes a while and blocks)
        /**
         *  @note After initialization, @p this is inserted into the
         *        available pool
         */
        void* initDH();

        /// main method of the client handler, performs handshake and then
        /// launches listener and shouter
        void* main();

        /// performs handshake protocol
        void handshake();

        /// listens for job messages from the client and adds them to the
        /// job queue
        void* listen();

        /// waits for jobs to complete and sends job completion messages back
        /// to the client
        void* shout();

        /// static method for pthreads, calls initDH()
        static void* dispatch_initDH( void* vp_h );

        /// static method for pthreads, calls handshake()
        static void* dispatch_main( void* vp_h );

        /// static method for pthreads, calls listen()
        static void* dispatch_listen( void* vp_h );

        /// static method for pthreads, calls shout()
        static void* dispatch_shout( void* vp_h );



    public:
        ClientHandler();
        ~ClientHandler();

        /// set the parent pointer and start DH parameter generation in
        /// detached thread
        void init( Pool_t*, Server*, ClientMap* );

        /// sec the client socket and start the handler interfacing with the
        /// client in a detached thread
        void handleClient( int sockfd, int termfd );

        /// adds a message to the outgoing queue
        void sendMessage( ClientMessage msg );

        /// retrieves a pointer to the inbound message queue for the handler
        InQueue_t* inboundQueue();



};



} // namespace server 
} // namespace filesystem
} // namespace openbook















#endif // HANDLER_H_

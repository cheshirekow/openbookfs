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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/ClientHandler.h
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

#include "ExceptionStream.h"
#include "MessageBuffer.h"
#include "Pool.h"
#include "Server.h"
#include "Synchronized.h"


namespace   openbook {
namespace filesystem {


class ClientException :
    public std::runtime_error
{
    public:
        ClientException( const std::string& msg ) throw():
            std::runtime_error(msg)
        {}

        virtual ~ClientException() throw(){}
};



class ClientHandler
{
    public:
        typedef ExceptionStream<ClientException> ex;
        typedef Pool<ClientHandler>                         Pool_t;
        typedef Synchronized< std::set< ClientHandler* > >  SyncedSet_t;

    private:
        static const unsigned int sm_bufsize = 256;

        Pool_t*             m_pool;             ///< pool to which this belongs
        SyncedSet_t*        m_active;           ///< set of active threads
        Server*             m_server;           ///< server configuration
        pthreads::Thread    m_thread;           ///< the thread we're running in

        pthreads::Thread    m_listenThread;     ///< child thread for listening
        pthreads::Thread    m_shoutThread;      ///< child thread for shouting
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
        void init( Pool_t*, SyncedSet_t*, Server* );

        /// sec the client socket and start the handler interfacing with the
        /// client in a detached thread
        void handleClient( int sockfd, int termfd );

        /// sends SIGINT to the main thread
        void kill();

        /// sends SIGINT to children
        void killChildren();



};



} // namespace filesystem
} // namespace openbook















#endif // HANDLER_H_

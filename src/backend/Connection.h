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
 *  @file   src/server/Connection.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_CONNECTION_H_
#define OPENBOOK_CONNECTION_H_

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
#include "Marshall.h"
#include "Pool.h"
#include "Queue.h"
#include "Synchronized.h"


namespace   openbook {
namespace filesystem {

// forward dec b/c Backend.h includes Connection.h
class Backend;

// forward dec b/c MessageHandler.h includes Connection.h
class MessageHandler;

/// dedicated RPC handler for a single client, manages both inbound and
/// outbound protocol buffer messages
class Connection
{
    public:
        typedef Pool<Connection>     Pool_t;
        typedef RefPtr<AutoMessage>  MsgPtr_t;
        typedef Queue< MsgPtr_t >    MsgQueue_t;

    private:
        static const unsigned int sm_bufsize = 256;

        Backend*            m_backend;          ///< top-level object
        uint32_t            m_peerId;           ///< id of peer connected to
        Pool_t*             m_pool;             ///< pool to which this belongs
        pthreads::Thread    m_thread;           ///< the thread we're running in

        MsgQueue_t          m_inboundMessages;  ///< received message queue
        MsgQueue_t          m_outboundMessages; ///< queue for messages to send

        pthreads::Thread    m_readThread;       ///< child thread for reading
        pthreads::Thread    m_writeThread;      ///< child thread for writing
        pthreads::Thread    m_workerThread;     ///< child thread for jobs

        MessageHandler*     m_worker;           ///< worker assigned to us

        pthreads::Mutex     m_mutex;            ///< locks this data
        Marshall            m_marshall;         ///< message/stream conversion
        int                 m_sockfd;           ///< socket for the connection

        // Diffie-Hellman Paramters
        CryptoPP::Integer               p,q,g;
        CryptoPP::SecByteBlock          m_cek;    ///< content encryption key
        CryptoPP::SecByteBlock          m_iv;     ///< initial vector
        CryptoPP::AutoSeededRandomPool  m_rng;    ///< random number gen

    public:
        Connection(Backend*);
        ~Connection();

        /// set the parent pointer and start DH parameter generation in
        /// detached thread
        void init( Pool_t* );

        /// set the client socket and start the handler interfacing with the
        /// client in a detached thread
        void handleClient( int sockfd, MessageHandler* worker );

    private:
        /// static method for pthreads, calls initDH()
        static void* dispatch_initDH( void* vp_h );

        /// static method for pthreads, calls handshake()
        static void* dispatch_main( void* vp_h );

        /// static method for pthreads, calls listen()
        static void* dispatch_listen( void* vp_h );

        /// static method for pthreads, calls shout()
        static void* dispatch_shout( void* vp_h );

        /// initialize Diffie Hellman paramters (takes a while and blocks)
        /**
         *  @note After initialization, @p this is inserted into the
         *        available pool
         */
        void initDH();

        /// returns this to the pool
        void returnToPool();

        /// main method of the client handler, performs handshake and then
        /// launches listener and shouter
        void main();

        /// performs handshake protocol whereby two peers agree on a shared
        /// key for this session, authenticate each other by public key, and
        /// authorize each other by table lookup
        void handshake();

        /// get pointer to inbound queue
        MsgQueue_t* inboundQueue(){ return &m_inboundMessages; }

        /// get pointer to outbound queue
        MsgQueue_t* outboundQueue(){ return &m_outboundMessages; }

    private:
        /// performs leader election in the handshake
        bool leaderElect();
        void keyExchange( CryptoPP::SecByteBlock& kek,
                          CryptoPP::SecByteBlock& mack );
        void sendCEK( CryptoPP::SecByteBlock& kek,
                      CryptoPP::SecByteBlock& mack );
        void recvCEK( CryptoPP::SecByteBlock& kek,
                      CryptoPP::SecByteBlock& mack );
        void authenticatePeer( std::string& base64 );

    public:
        /// listens for job messages from the client and adds them to the
        /// job queue
        void listen();

        /// waits for jobs to complete and sends job completion messages back
        /// to the client
        void shout();




};



} // namespace filesystem
} // namespace openbook















#endif // Connection_H_

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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/ServerHandler.h
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_SERVERHANDLER_H_
#define OPENBOOK_SERVERHANDLER_H_


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

#include "Client.h"
#include "ExceptionStream.h"
#include "MessageBuffer.h"
#include "Queue.h"



namespace   openbook {
namespace filesystem {


class ServerException :
    public std::runtime_error
{
    public:
        ServerException( const std::string& msg ) throw():
            std::runtime_error(msg)
        {}

        virtual ~ServerException() throw(){}
};



class ServerHandler
{
    public:
        typedef ExceptionStream<ServerException> ex;
        typedef Queue<TypedMessage>              MsgQueue_t;

    private:
        static const unsigned int sm_bufsize = 256;

        Client*             m_client;           ///< server configuration
        pthreads::Thread    m_thread;           ///< the thread we're running in

        MsgQueue_t*         m_inboundMessages;  ///< received message queue
        MsgQueue_t          m_outboundMessages;    ///< messages to send

        pthreads::Thread    m_listenThread;     ///< child thread for listening
        pthreads::Thread    m_shoutThread;      ///< child thread for shouting
        pthreads::Mutex     m_mutex;            ///< locks this data
        MessageBuffer       m_msg;              ///< message buffer
        int                 m_fd[2];            ///< socket and terminator fd
        bool                m_shouldDie;        ///< signals that it's time to
                                                ///  shutdown

        // Diffie-Hellman Paramters
        CryptoPP::Integer   p,q,g;

        CryptoPP::SecByteBlock          m_cek;    ///< content encryption key
        CryptoPP::SecByteBlock          m_iv;     ///< initial vector
        CryptoPP::AutoSeededRandomPool  m_rng;    ///< random number gen

        std::string                 m_rsaPubStr;
        CryptoPP::RSA::PublicKey    m_rsaPubKey;
        CryptoPP::RSA::PrivateKey   m_rsaPrivKey;


        /// main method of the server handler, creates connection, performs
        /// hanshake, then launches listener and shouter
        void* main();

        /// connect to server
        void createConnection();

        /// performs handshake protocol
        void handshake();

        /// listens for job messages from the client and adds them to the
        /// job queue
        void* listen();

        /// waits for jobs to complete and sends job completion messages back
        /// to the client
        void* shout();

        /// static method for pthreads, calls handshake()
        static void* dispatch_main( void* vp_h );

        /// static method for pthreads, calls listen()
        static void* dispatch_listen( void* vp_h );

        /// static method for pthreads, calls shout()
        static void* dispatch_shout( void* vp_h );



    public:
        ServerHandler();
        ~ServerHandler();

        /// set the parent pointer and terminator pipe fd
        void init( Client*, MsgQueue_t*, int termfd );

        /// start the handler
        void start();

        /// join the handler
        void join();

        /// job sink
        void sendMessage( TypedMessage msg );

};



} // namespace filesystem
} // namespace openbook














#endif // SERVERHANDLER_H_

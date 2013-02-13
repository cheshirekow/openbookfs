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

#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <cpp-pthreads.h>
#include <vector>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/dh.h>
#include <crypto++/dh2.h>

#include "ExceptionStream.h"
#include "MessageBuffer.h"
#include "Pool.h"
#include "SelectSet.h"


namespace   openbook {
namespace filesystem {


class RequestException :
    public std::runtime_error
{
    public:
        RequestException( const std::string& msg ) throw():
            std::runtime_error(msg)
        {}

        virtual ~RequestException() throw(){}
};



class RequestHandler
{
    public:
        typedef ExceptionStream<RequestException> ex;

    private:
        typedef Pool<RequestHandler>    Pool_t;
        static const unsigned int sm_bufsize = 256;

        Pool_t*             m_pool;             ///< pool to which this belongs
        pthreads::Thread    m_thread;           ///< the thread we're running in
        pthreads::Mutex     m_mutex;            ///< locks this data
        MessageBuffer       m_msg;              ///< message buffer
        int                 m_sock;             ///< socket fd

        CryptoPP::AutoSeededRandomPool  m_rng;    ///< random number gen

        CryptoPP::DH                m_dh;         ///< Diffie-Hellman structure
        CryptoPP::DH2               m_dh2;        ///< Diffie-Hellman structure
        CryptoPP::SecByteBlock      m_spriv;      ///< static private key
        CryptoPP::SecByteBlock      m_spub;       ///< static public key
        CryptoPP::SecByteBlock      m_epriv;      ///< ephemeral private key
        CryptoPP::SecByteBlock      m_epub;       ///< ephemeral public key
        CryptoPP::SecByteBlock      m_shared;     ///< shared key

        void cleanup();

    public:
        RequestHandler();
        ~RequestHandler();

        /// set the parent pointer and start DH parameter generation in
        /// detached thread
        void init( Pool_t* );

        /// initialize Diffie Hellman paramters (takes a while and blocks)
        void initDH();

        /// start the handler interfacing with the client on the socket
        void start( int sockfd );

        /// makes this a callable object that can be sent to the thread
        void* operator()();

};



} // namespace filesystem
} // namespace openbook















#endif // HANDLER_H_

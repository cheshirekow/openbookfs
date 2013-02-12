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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/RequestHandler.cpp
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <errno.h>
#include <sstream>

#include <protobuf/message.h>
#include <protobuf/io/zero_copy_stream_impl.h>
#include <crypto++/files.h>

#include "RequestHandler.h"
#include "messages.h"
#include "messages.pb.h"

namespace   openbook {
namespace filesystem {


void RequestHandler::cleanup()
{
    close(m_sock);
    // when finished we put ourselves back in the thread pool
    std::cout << "Handler " << (void*) this << " is returning to the pool"
              << std::endl;
    m_pool->reassign(this);
}

RequestHandler::RequestHandler():
    m_pool(0)
{

}

RequestHandler::~RequestHandler()
{

}

void RequestHandler::init(Pool_t* pool)
{
    m_pool = pool;
}

void RequestHandler::setKeys( const std::string& pub,
                        const CryptoPP::RSA::PrivateKey& priv )
{
    m_serverPub = pub;
    m_serverKey = priv;
}

void RequestHandler::start( int sockfd )
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);
        m_sock = sockfd;

        Attr<Thread> attr;
        attr.init();
        attr << DETACHED;
        int result = m_thread.launch(attr,this);
        attr.destroy();

        if( result )
        {
            std::cerr << "Failed to start handler thread: ";
            switch(result)
            {
                case EAGAIN:
                    std::cerr << "EAGAIN";
                    break;

                case EINVAL:
                    std::cerr << "EINVAL";
                    break;

                case EPERM:
                    std::cerr << "EPERM";
                    break;

                default:
                    std::cerr << "unexpected errno";
                    break;
            }
            std::cerr << std::endl;
            m_pool->reassign(this);
        }
    }
}

void* RequestHandler::operator()()
{
    using namespace pthreads;
    ScopedLock lock(m_mutex);

    int received = 0;

    //create message buffers
    std::cerr << "handler " << (void*) this << " is starting up"
              << std::endl;

    try
    {

    // first message must be an auth request
    char type = m_msg.read(m_sock,m_serverKey,m_rng);
    if( type != MSG_AUTH_REQ )
        ex()() << "Received message type : " << (int)type
               << " when expecting MSG_AUTH_REQ, terminating"
                  " client";

    // read the client's public key
    messages::AuthRequest* authReq = m_msg.msg<MSG_AUTH_REQ>();
    std::stringstream  inkey( authReq->public_key() );
    CryptoPP::FileSource keyFile(inkey,true);
    CryptoPP::ByteQueue  queue;
    keyFile.TransferTo(queue);
    queue.MessageEnd();
    m_clientKey.Load(queue);

    int authRetry = 0;
    for(; authRetry < 3; authRetry++)
    {
        // generate a random string
        std::string randomStr;
        randomStr.resize(30);
        m_rng.GenerateBlock( (unsigned char*)&(randomStr[0]), 30 );

        messages::AuthChallenge* challenge
            = m_msg.msg<MSG_AUTH_CHALLENGE>();

        // for now, let's pretend that all clients are authorized and we'll
        // just verify that they are the key owner
        challenge->set_public_key(m_serverPub);
        challenge->set_challenge(randomStr);
        challenge->set_req_id(1);
        challenge->set_type( messages::AuthChallenge::AUTHENTICATE );

        // now send the message
        m_msg.write(m_sock,MSG_AUTH_CHALLENGE,m_clientKey,m_rng);

        // and get the reply
        type = m_msg.read(m_sock,m_serverKey,m_rng);

        // the type must be a challenge reply
        if( type != MSG_AUTH_SOLN )
        {
            std::cerr << "Client responded to authentication challenge with "
                         "the wrong message (id: " << (int) type  << "), "
                         "resending challenge";
            continue;
        }

        messages::AuthSolution* soln = m_msg.msg<MSG_AUTH_SOLN>();
        if( soln->solution().compare(randomStr) !=  0 )
        {
            std::cerr << "Client failed the challenge, sending a new "
                         "challenge";
            continue;
        }

        std::cout << "client successfully authenticated" << std::endl;
        messages::AuthResult* result = m_msg.msg<MSG_AUTH_RESULT>();
        result->set_req_id(2);
        result->set_response(true);

        m_msg.write(m_sock,MSG_AUTH_RESULT,m_clientKey,m_rng);
        break;
    }

    // if too many retries
    if( authRetry >= 3 )
    {
        std::cout << "client failed too many times" << std::endl;
        messages::AuthResult* result = m_msg.msg<MSG_AUTH_RESULT>();
        result->set_req_id(2);
        result->set_response(false);

        m_msg.write(m_sock,MSG_AUTH_RESULT,m_clientKey,m_rng);
        cleanup();
        return 0;
    }


    while(1)
    {
        type = m_msg.read(m_sock,m_serverKey,m_rng);
        std::cout << "Received message of type " << (int) type << std::endl;
    }

    }
    catch ( std::exception& ex )
    {
        std::cerr << ex.what() << std::endl;
        cleanup();
        return 0;
    }

    return 0;
}











} // namespace filesystem
} // namespace openbook

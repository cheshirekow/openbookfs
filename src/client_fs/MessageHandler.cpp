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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/MessageHandler.cpp
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "MessageHandler.h"

namespace   openbook {
namespace filesystem {


void* MessageHandler::dispatch_main( void* vp_h )
{
    MessageHandler* h = static_cast<MessageHandler*>(vp_h);
    return h->main();
}

void* MessageHandler::main()
{
    std::cout << "Message handler " << (void*)this << " starting up\n";

    bool shouldDie = false;
    while(!shouldDie)
    {
        TypedMessage msg;

        // wait for a new message
        m_msgQueue->extract(msg);

        // handle the message
        switch( msg.type )
        {
            case MSG_QUIT:
            {
                shouldDie = true;
                std::cout << "Message handler " << (void*)this
                          << " shutting down \n";
                break;
            }

            case MSG_PING:
            {
                std::cout << "Message handler " << (void*)this
                          << " got a PING \n";
                TypedMessage out;
                out.type = MSG_PONG;

                messages::Pong* pong= new messages::Pong();
                pong->set_payload(0xb19b00b5);
                out.msg = pong;

                sleep(1);
                m_server->sendMessage(out);
                break;
            }

            case MSG_PONG:
            {
                std::cout << "Message handler " << (void*)this
                          << " got a PONG \n";
                TypedMessage out;
                out.type = MSG_PING;

                messages::Ping* ping= new messages::Ping();
                ping->set_payload(0xb19b00b5);
                out.msg = ping;

                sleep(1);
                m_server->sendMessage(out);
                break;
            }

            default:
            {
                std::cerr << "Message handler for type (" << (int)msg.type
                          << ") : " << messageIdToString(msg.type)
                          << "isn't implemented";
                break;
            }
        }

        if(msg.msg)
            delete msg.msg;
    }

    m_pool->reassign(this);
    return 0;
}

MessageHandler::MessageHandler():
    m_pool(0),
    m_msgQueue(0)
{
    m_mutex.init();
}

MessageHandler::~MessageHandler()
{
    m_mutex.destroy();
}

void MessageHandler::init(
        Pool_t*         pool,
        MsgQueue_t*     queue,
        Client*         client,
        ServerHandler*  server )
{
    m_pool     = pool;
    m_msgQueue = queue;
    m_client   = client;
    m_server   = server;
}

void MessageHandler::start()
{
    // lock scope
    // the worker thread wont be able to start doing anything until
    // we're done in here
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);

        Attr<Thread> attr;
        attr.init();
        attr << DETACHED;
        int result = m_thread.launch(attr,dispatch_main,this);
        attr.destroy();

        if( result )
        {
            std::cerr << "Failed to start job handler thread, errno " << result
                      << " : " << strerror(result) << std::endl;
            m_pool->reassign(this);
        }
    }
}





} // namespace filesystem
} // namespace openbook

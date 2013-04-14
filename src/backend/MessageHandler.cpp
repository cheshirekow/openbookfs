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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/MessageHandler.cpp
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include <iostream>
#include "Backend.h"
#include "MessageHandler.h"




namespace   openbook {
namespace filesystem {

MessageHandler::MessageHandler()
{
    m_backend       = 0;
    m_pool          = 0;
    m_inboundQueue  = 0;
    m_outboundQueue = 0;
    m_shouldQuit    = false;
    m_mutex.init();
}

MessageHandler::~MessageHandler()
{
    m_mutex.destroy();
}

void MessageHandler::init(Backend* backend, Pool_t* pool)
{
    m_backend = backend;
    m_pool    = pool;
}

void MessageHandler::go( pthreads::Thread& thread, MsgQueue_t* in, MsgQueue_t* out )
{
    m_inboundQueue  = in;
    m_outboundQueue = out;
    thread.launch(dispatch_main,this);
}

void MessageHandler::returnToPool()
{
    m_inboundQueue  = 0;
    m_outboundQueue = 0;
    m_pool->reassign(this);
}

void MessageHandler::main()
{
    std::cout << "Message Handler " << (void*)this << "Starting up\n";

    MsgPtr_t msg;
    while(!m_shouldQuit)
    {
        m_inboundQueue->extract(msg);
        MessageDispatcher<0,NUM_MSG-2>::dispatch( this, msg );
    }

    std::cout << "Message Handler " << (void*)this << "Shutting down\n";
}

void* MessageHandler::dispatch_main(void* vp_h )
{
    MessageHandler* handler = static_cast<MessageHandler*>(vp_h);
    handler->main();
    return vp_h;
}



void MessageHandler::handleMessage( messages::Quit* msg )               { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::LeaderElect* msg )        { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::DiffieHellmanParams* msg ){ exceptMessage(msg); }
void MessageHandler::handleMessage( messages::KeyExchange* msg )        { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::ContentKey* msg )         { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthRequest* msg )        { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthChallenge* msg )      { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthSolution* msg )       { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthResult* msg )         { exceptMessage(msg); }

void MessageHandler::handleMessage( messages::Ping*         msg )
{

}

void MessageHandler::handleMessage( messages::Pong*         msg )
{

}


void MessageHandler::handleMessage( messages::Subscribe*    msg )
{

}


void MessageHandler::handleMessage( messages::Unsubscribe*  msg )
{

}


void MessageHandler::handleMessage( messages::NewVersion*   msg )
{

}


void MessageHandler::handleMessage( messages::RequestFile*  msg )
{

}


void MessageHandler::handleMessage( messages::FileInfo*     msg )
{

}


void MessageHandler::handleMessage( messages::FileChunk*    msg )
{

}


void MessageHandler::handleMessage( messages::DirInfo*      msg )
{

}


void MessageHandler::handleMessage( messages::DirChunk*     msg )
{

}


void MessageHandler::handleMessage( messages::Invalid*      msg )
{

}





} // namespace filesystem
} // namespace openbook

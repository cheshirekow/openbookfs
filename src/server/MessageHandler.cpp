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


#include <sys/file.h>

#include <boost/filesystem.hpp>

#include "MessageHandler.h"
#include "MetaFile.h"

/// simple macro used only in MessageHandler.cpp which creates a case statement
/// for a message type and calls the appropriate handleMessage() overload with
/// an upcast message pointer
#define MSG_HANDLE(x)   \
    case x:             \
    {                   \
        handleMessage(msg, message_cast<x>(msg.typed.msg)); \
        break;          \
    }

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
        ClientMessage msg;

        // wait for a new message
        m_msgQueue->extract(msg);

        // handle the message
        switch( msg.typed.type )
        {
            case MSG_QUIT:
            {
                shouldDie = true;
                std::cout << "Message handler " << (void*)this
                          << " shutting down \n";
                break;
            }

            MSG_HANDLE(MSG_PING)
            MSG_HANDLE(MSG_PONG)
            MSG_HANDLE(MSG_NEW_VERSION)

            default:
            {
                std::cerr << "Message handler for type ("
                          << (int)msg.typed.type
                          << ") : " << messageIdToString(msg.typed.type)
                          << "isn't implemented";
                break;
            }
        }

        if(msg.typed.msg)
            delete msg.typed.msg;
    }

    m_pool->reassign(this);
    return 0;
}

void MessageHandler::handleMessage(
        ClientMessage msg, messages::Ping* upcast )
{
    std::cout << "Message handler " << (void*)this << " got a PING \n";
    ClientMessage out = msg;
    out.typed.type = MSG_PONG;

    messages::Pong* pong= new messages::Pong();
    pong->set_payload(0xb19b00b5);
    out.typed.msg = pong;
    sleep(1);
    out.send();
}

void MessageHandler::handleMessage(
        ClientMessage msg, messages::Pong* upcast )
{
    std::cout << "Message handler " << (void*)this
              << " got a PONG \n";
    ClientMessage out = msg;
    out.typed.type = MSG_PING;

    messages::Ping* ping= new messages::Ping();
    ping->set_payload(0xb19b00b5);
    out.typed.msg = ping;
    sleep(1);
    out.send();
}

void MessageHandler::handleMessage(
        ClientMessage msg, messages::NewVersion* upcast )
{
    namespace fs = boost::filesystem;

    // path to the file
    fs::path filePath = fs::path(m_server->rootDir()) / upcast->path();
    fs::path metaPath = fs::path(m_server->rootDir()) / (upcast->path() + ".obfsmeta");

    try
    {
        // if the file doesn't exist, create it (create message may have
        // been reordered to come after an update message and we dont
        // want to fail on the update)
        if( !fs::exists(filePath) )
        {
            ::close(::open( filePath.c_str(),
                            O_RDWR | O_CREAT, S_IRUSR | S_IWUSR ));
            MetaData meta(metaPath.string());
            meta.subscribe(msg.client_id);
            meta.create();
        }

        // load the meta file
        MetaData meta(metaPath.string());
        meta.load();

        // if the file is synced then we can make the sender the owner
        // and start downloading it
        if( meta.state() == meta::SYNCED )
        {
            // check to make sure that the client started with the
            // correct version
            if( upcast->base_version() == meta.baseVersion() )
            {
                meta.set_state( meta::INCOMPLETE );
                meta.set_owner( msg.client_id );
                meta.set_clientVersion( upcast->client_version() );
                // todo: send message to start download
            }
            else
            {
                // todo: ignore, file is in conflict for the client
                // who sent the message
            }
        }

        // if the state is downloading but the owner is the client who
        // sent this message then we can start downloading a new version
        else
        {
            if( meta.owner() == msg.client_id )
            {
                meta.set_clientVersion(upcast->client_version());
                // todo: send message to restart download
            }
            else
            {
                // todo: ignore, file is in conflict for the client
                // who sent the message
            }
        }

        meta.flush();
    }
    catch( std::exception& ex )
    {
        std::cerr << "Error handling a new version message " << ex.what()
                  << std::endl;
    }
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
        Server*         server )
{
    m_pool     = pool;
    m_msgQueue = queue;
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

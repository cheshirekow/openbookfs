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


#include <cstring>
#include <sys/file.h>
#include <sys/mman.h>

#include "MessageHandler.h"
#include "ExceptionStream.h"
#include "MetaFile.h"


/// simple macro used only in MessageHandler.cpp which creates a case statement
/// for a message type and calls the appropriate handleMessage() overload with
/// an upcast message pointer
#define MSG_HANDLE(x)   \
    case x:             \
    {                   \
        handleMessage(msg, message_cast<x>(msg.msg)); \
        break;          \
    }

namespace   openbook {
namespace filesystem {
namespace     client {



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

            MSG_HANDLE(MSG_PING)
            MSG_HANDLE(MSG_PONG)
            MSG_HANDLE(MSG_REQUEST_CHUNK)
            MSG_HANDLE(MSG_COMMIT)

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

void MessageHandler::handleMessage(
        TypedMessage msg, messages::Ping* upcast )
{
    std::cout << "Message handler " << (void*)this << " got a PING \n";
    TypedMessage out;
    out.type = MSG_PONG;

    messages::Pong* pong= new messages::Pong();
    pong->set_payload(0xb19b00b5);
    out.msg = pong;

    sleep(5);
    m_server->sendMessage(out);
}

void MessageHandler::handleMessage(
        TypedMessage msg, messages::Pong* upcast )
{
    std::cout << "Message handler " << (void*)this << " got a PONG \n";
    TypedMessage out;
    out.type = MSG_PING;

    messages::Ping* ping= new messages::Ping();
    ping->set_payload(0xb19b00b5);
    out.msg = ping;

    sleep(5);
    m_server->sendMessage(out);
}

void MessageHandler::handleMessage(
        TypedMessage msg, messages::RequestChunk* upcast )
{
    namespace fs = boost::filesystem;

    std::cout << "Handling request for file chunk"
              << "\n      path: " << upcast->path()
              << "\n   version: " << upcast->base_version()
                                  << "." << upcast->client_version()
              << "\n    offset: " << upcast->offset()
              << "\n      size: " << upcast->size()
              << std::endl;

    fs::path filePath = fs::path(m_client->realRoot()) / (upcast->path());
    fs::path metaPath = fs::path(m_client->realRoot()) / (upcast->path() + ".obfsmeta");

    // create our response object
    messages::FileChunk* chunk = new messages::FileChunk();
    chunk->set_path(   upcast->path() );
    chunk->set_msg_id( upcast->msg_id() );
    chunk->set_offset( upcast->offset() );

    // first open and lock the meta data file, this will prevent file
    // deletion, modification, update while we're working
    try
    {
        MetaData meta(metaPath);
        meta.load();
        chunk->set_client_version(meta.clientVersion());

        // if the current version is higher than the requested version then
        // we need to reply without data
        if( upcast->client_version() == meta.clientVersion() )
        {
            int fd = ::open( filePath.c_str(), O_RDWR );
            if( fd < 0 )
            {
                meta.flush();
                ex()() << "Failed to open file for reading a chunk: "
                          << filePath << " (" << errno << ") : "
                          << strerror(errno);
            }

            std::cout << "Opened the file" << std::endl;

            off_t   off       = upcast->offset();           //< requested offset
            off_t   pagesize  = sysconf(_SC_PAGE_SIZE);     //< system page size
            off_t   off_n     = pagesize * (off/pagesize);  //< round down
            off_t   extra     = off - off_n;
            off_t   size      = upcast->size() + extra;

            // map the file
            void* ptr = mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,off_n);
            if( ptr == MAP_FAILED )
            {
                meta.flush();
                ex()() << "Failed to map file " << filePath << " for reading "
                       << "a chunk";
            }

            std::cout << "Mapped the file" << std::endl;

            // copy contents into the message
            chunk->set_data( (char*)ptr + extra, upcast->size() );

            std::cout << "Copied the data" << std::endl;

            // unmap and close the file
            munmap(ptr,upcast->size());
            ::close(fd);
        }
        meta.flush();
    }
    catch( std::exception& ex )
    {
        std::cerr << "Failed to deliver file chunk: " << ex.what();
    }

    // send the message
    TypedMessage out( MSG_FILE_CHUNK, chunk );
    m_server->sendMessage(out);

    std::cout << "Sent reply message" << std::endl;
}



void MessageHandler::handleMessage(
        TypedMessage msg, messages::Commit* upcast )
{
    namespace fs = boost::filesystem;

    std::cout << "Handling commit of file " << upcast->path() << std::endl;

    fs::path filePath = fs::path(m_client->realRoot()) / (upcast->path());
    fs::path metaPath = fs::path(m_client->realRoot()) / (upcast->path() + ".obfsmeta");

    // first open and lock the meta data file, this will prevent file
    // deletion, modification, update while we're working
    try
    {
        MetaData meta(metaPath);
        meta.load();

        // make sure this isn't a repeat message
        if( upcast->base_version() != meta.baseVersion() )
        {
            meta.flush();
            ex()() << "Received a commit message from "
                   << upcast->base_version() << "." << upcast->client_version()
                   << " to " << upcast->new_version() << ".0 but the client "
                   << "version is " << meta.baseVersion() << "."
                   << meta.clientVersion() << " so ignoring the message ";
        }

        meta.set_baseVersion(upcast->new_version());
        meta.set_clientVersion( meta.clientVersion()
                                - upcast->client_version() );

        std::cout << "Updating meta data for " << filePath
                  << "\n     base: " << meta.baseVersion()
                  << "\n   client: " << meta.clientVersion()
                  << std::endl;
        meta.flush();
    }
    catch( std::exception& ex )
    {
        std::cerr << "Failed to commit new version: " << ex.what();
    }

    // todo: send ack
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





} // namespace client
} // namespace filesystem
} // namespace openbook

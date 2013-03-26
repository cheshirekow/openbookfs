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
 *  @file   src/client_fs/MessageHandler.cpp
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include <sys/file.h>

#include <boost/filesystem.hpp>

#include "ClientHandler.h"
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
 namespace    server {


void* MessageHandler::dispatch_main( void* vp_h )
{
    MessageHandler* h = static_cast<MessageHandler*>(vp_h);
    return h->main();
}

void* MessageHandler::main()
{
    std::cout << "Message handler " << (void*)this << " starting up\n";

    pthreads::ScopedLock lock(m_mutex);
    MsgQueue_t* msgQueue = m_client->inboundQueue();

    bool shouldDie = false;
    while(!shouldDie)
    {
        ClientMessage msg;

        // wait for a new message
        msgQueue->extract(msg);

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
            MSG_HANDLE(MSG_FILE_CHUNK)

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

    return 0;
}

void MessageHandler::handleMessage(
        ClientMessage msg, messages::Ping* upcast )
{
    std::cout << "Message handler " << (void*)this << " got a PING \n";
    ClientMessage out = msg;
    out.typed.type = MSG_PONG;

    messages::Pong* pong= new messages::Pong();
    pong->set_payload(0xdeadf00d);
    out.typed.msg = pong;
    sleep(5);
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
    ping->set_payload(0xdeadf00d);
    out.typed.msg = ping;
    sleep(5);
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
        // open the meta file. This will create it if it doesn't already
        // exist
        MetaData meta(metaPath.string());
        meta.load();

        // if the file doesn't exist, create it (create message may have
        // been reordered to come after an update message and we dont
        // want to fail on the update)
        if( !fs::exists(filePath) )
        {
            ::close(::open( filePath.c_str(),
                            O_RDWR | O_CREAT, S_IRUSR | S_IWUSR ));
        }

        // if this is the create message we are finished
        if( upcast->size() == 0 )
        {
            meta.flush();
            return;
        }

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

                // truncate the file to prepare for download
                int result = ::truncate(filePath.c_str(),upcast->size());
                if( result < 0 )
                    ex()() << "Failed to truncate file " << filePath
                           << " to size " << upcast->size()
                           << " in preparation for download ";

                /// chunk size is at most 100kB
                uint64_t size = std::min((long int)upcast->size(), 102400L);

                messages::RequestChunk* req = new messages::RequestChunk();
                req->set_path( upcast->path() );
                req->set_offset(0);
                req->set_size(size);
                req->set_base_version(meta.baseVersion());
                req->set_client_version(meta.clientVersion());
                req->set_msg_id(0); // todo: set message id

                ClientMessage out = msg;
                out.typed.type = MSG_REQUEST_CHUNK;
                out.typed.msg  = req;
                out.send();
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

                // truncate the file to prepare for download
                int result = ::truncate(filePath.c_str(),upcast->size());
                if( result < 0 )
                    ex()() << "Failed to truncate file " << filePath
                           << " to size " << upcast->size()
                           << " in preparation for download ";

                /// chunk size is at most 100kB
                uint64_t size = std::min((long int)upcast->size(), 102400L);

                messages::RequestChunk* req = new messages::RequestChunk();
                req->set_path( upcast->path() );
                req->set_offset(0);
                req->set_size(size);
                req->set_base_version(meta.baseVersion());
                req->set_client_version(meta.clientVersion());
                req->set_msg_id(0); // todo: set message id

                ClientMessage out = msg;
                out.typed.type = MSG_REQUEST_CHUNK;
                out.typed.msg  = req;
                out.send();
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

void MessageHandler::handleMessage(
        ClientMessage msg, messages::FileChunk* upcast )
{
    namespace fs = boost::filesystem;

    fs::path filePath = fs::path( m_server->rootDir() ) / upcast->path();
    fs::path metaPath = fs::path( m_server->rootDir() )
                            / ( upcast->path() + ".obfsmeta");

    std::cout << "received a file chunk:"
              << "\n    path: " << upcast->path()
              << "\n version: " << upcast->client_version()
              << "\n  offset: " << upcast->offset()
              << "\n    size: " << upcast->data().size()
              << std::endl;

    uint64_t fileSize = 0;
    // open lock and load the meta file
    try
    {
        MetaData meta(metaPath.c_str());
        meta.load();

        // check to make sure that the transfer hasn't been pre-empted
        if( msg.client_id != meta.owner() )
        {
            meta.flush();
            ex()() << "It's an old message from client " << msg.client_id
                   << " because current owner is " << meta.owner();
        }

        // check to make sure that the client still has the same file
        if( upcast->base_version() != meta.baseVersion() ||
                upcast->client_version() != meta.clientVersion() )
        {
            meta.flush();
            ex()() << "Received chunk of file version "
                  << upcast->base_version() << "." << upcast->client_version()
                  << " but the server version is at "
                  << meta.baseVersion() << "." << meta.clientVersion();
        }

        // open the file for writing
        int fd = ::open( filePath.c_str(), O_RDWR );
        if( fd < 0 )
        {
            meta.flush();
            ex()() << "Failed to open file for writing (" << errno << ") : "
                   << strerror(errno);
        }

        // stat the file for its size
        struct stat fs;
        int result = ::fstat(fd,&fs);
        if( result < 0 )
        {
            ::close(fd);
            meta.flush();
            ex()() << "Failed to stat the file for size (" << errno << ") : "
                   << strerror(errno );
        }

        fileSize = fs.st_size;

        // seek to the offset
        result = ::lseek(fd, upcast->offset(), SEEK_SET );
        if( result == (off_t)-1 )
        {
            ::close(fd);
            meta.flush();
            ex()() << "Failed to seek to offset " << upcast->offset()
                   << "for writing (" << errno << ") : "
                   << strerror(errno);
        }

        // do the write
        const std::string& buf = upcast->data();
        result = ::write(fd,buf.c_str(),buf.size());
        if( result < buf.size() )
        {
            std::cerr << "Unsuccessful write of size: " << buf.size()
                      << " to " << filePath << " (" << errno << ") : "
                      << strerror(errno) << std::endl;
        }

        // close the file
        ::close(fd);

        // if this is the last request then we can mark the file synchronized
        if( upcast->offset() + upcast->data().size() == fileSize )
        {
            meta.set_state(meta::SYNCED);

            messages::Commit* commit = new messages::Commit();
            commit->set_msg_id(0); // todo: set message id
            commit->set_base_version(meta.baseVersion());
            commit->set_client_version(meta.clientVersion());

            meta.set_baseVersion(meta.baseVersion()+1);
            meta.set_clientVersion(0);

            commit->set_new_version(meta.baseVersion());
            commit->set_path( upcast->path() );

            ClientMessage out = msg;
            out.typed.type = MSG_COMMIT;
            out.typed.msg  = commit;
            out.send();

            // todo: send notification to subscribed clients
        }
        // otherwise request the next chunk
        else
        {
            uint64_t newOff = upcast->offset() + upcast->data().size();
            uint64_t left   = fileSize - newOff;
            uint64_t size   = std::min( left, (uint64_t)102400UL );

            messages::RequestChunk* req = new messages::RequestChunk();
            req->set_base_version(meta.baseVersion());
            req->set_client_version(meta.clientVersion());
            req->set_msg_id(0); // todo: set message id
            req->set_offset( newOff );
            req->set_size( size );
            req->set_path( upcast->path() );

            ClientMessage out = msg;
            out.typed.type = MSG_REQUEST_CHUNK;
            out.typed.msg  = req;
            out.send();
        }

        // flush meta data and release the lock
        meta.flush();
    }
    catch( std::exception& ex )
    {
        std::cerr << "Failed to blit file chunk: " << ex.what() << std::endl;
    }
}

MessageHandler::MessageHandler():
    m_client(0)
{
    m_mutex.init();
}

MessageHandler::~MessageHandler()
{
    m_mutex.destroy();
}

void MessageHandler::init(
        Server*         server,
        ClientHandler*  client)
{
    m_server   = server;
    m_client   = client;
}







} // namespace server 
} // namespace filesystem
} // namespace openbook

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
 *  @file   src/backend/MessageHandler.cpp
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include <iostream>
#include <boost/format.hpp>
#include "Backend.h"
#include "MessageHandler.h"
#include "Marshall.h"
#include "VersionVector.h"
#include "jobs/PingJob.h"
#include "jobs/SendTree.h"
#include "jobs/SendFile.h"


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
    returnToPool();
}

void MessageHandler::go( int peerId, MsgQueue_t* in, MsgQueue_t* out )
{
    m_inboundQueue  = in;
    m_outboundQueue = out;
    m_peerId        = peerId;
    m_shouldQuit    = false;
    main();
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
        MsgSwitch::dispatch( this, msg );
    }

    std::cout << "Message Handler " << (void*)this << "Shutting down\n";
}

void MessageHandler::mapVersion( const VersionVector& v_in, VersionVector& v_out )
{
    std::stringstream report;
    report << "MessageHandler::mapVersion():\n";
    for( auto& pair : v_in )
    {
        v_out[ m_peerMap[pair.first] ] = pair.second;
        report << boost::format("   %d -> %d : %d\n")
                    % pair.first
                    % m_peerMap[pair.first]
                    % pair.second;
    }
    if( v_out.find(0) == v_out.end() )
        v_out[0] = 0;
    std::cout << report.str();
}


void MessageHandler::handleMessage( messages::LeaderElect* msg )        { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::DiffieHellmanParams* msg ){ exceptMessage(msg); }
void MessageHandler::handleMessage( messages::KeyExchange* msg )        { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::ContentKey* msg )         { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthRequest* msg )        { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthChallenge* msg )      { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthSolution* msg )       { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::AuthResult* msg )         { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::UserInterfaceReply*  msg) { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::PeerList* msg )           { exceptMessage(msg); }
void MessageHandler::handleMessage( messages::MountList*  msg)          { exceptMessage(msg); }

void MessageHandler::handleMessage( messages::SetDisplayName* msg)
{
    m_backend->setDisplayName(
            msg->displayname());

    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    std::stringstream strm;
    strm << "Changed display name to: " << msg->displayname();
    reply->set_msg(strm.str());
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::SetDataDir* msg)
{
    m_backend->setDataDir(
            msg->datadir() );

    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::SetLocalSocket* msg)
{
    m_backend->setLocalSocket(
            msg->port());

    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::SetRemoteSocket* msg)
{
    m_backend->setRemoteSocket(
            msg->addressfamily(),
            msg->node(),
            msg->service() );

    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::SetClientSocket* msg)
{
    m_backend->setClientSocket(
            msg->addressfamily(),
            msg->node() );

    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::SetMaxConnections* msg)
{
    m_backend->setMaxConnections(
            msg->maxconn() );

    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::LoadConfig* msg)
{
    m_backend->loadConfig(
            msg->filename() );

    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::SaveConfig* msg)
{
    m_backend->saveConfig(
            msg->filename() );

    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::AttemptConnection* msg )
{
    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();

    try
    {
        m_backend->attemptConnection(
                msg->isremote(),
                msg->node(),
                msg->service() );
        reply->set_ok(true);
    }
    catch (const std::exception& ex)
    {
        reply->set_ok(false);
        reply->set_msg(ex.what());
    }

    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::AddMountPoint* msg)
{
    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();

    const int   nargs =100;
    const int   nchars=256;
    char        argBuf[nchars]; //< buffer for arguments
    int         argw = 0;       //< write offset
    char*       argv[nargs];    //< argument index
    int         argc = 0;       //< number of arguments

    // zero out contents for ease of debugging, and to implicitly
    // set null terminals for strings
    memset(argBuf,0,sizeof(argBuf));
    memset(argv,0,sizeof(argv));

    char*   pwrite = argBuf; //< write head
    // for each argument in the sequence
    for(int i=0; i < msg->argv_size(); i++)
    {
        // retrieve the argument into a string
        std::string arg = msg->argv(i);

        // number of chars left in buffer
        int remainder = nchars - (pwrite - argBuf);

        // if we dont have room for the argument then quit
        if( arg.length() + 1 > remainder )
            break;

        argv[argc++] = pwrite;  //< point the j'th argument to
                                //  current write head
        arg.copy(pwrite,arg.length(),0);    //< copy the argument
        pwrite += arg.length() + 1;   //< advance the write head
    }

    try
    {
        m_backend->mount(
                msg->path(),
                msg->relpath(),
                argc,
                argv);
        reply->set_ok(true);
    }
    catch( const std::exception& ex)
    {
        reply->set_ok(false);
        reply->set_msg(ex.what());
    }

    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::RemoveMountPoint* msg)
{
    // for just send an empty OK response
    messages::UserInterfaceReply* reply = new messages::UserInterfaceReply();
    reply->set_ok(true);

    try
    {
        m_backend->unmount( msg->imp() );
    }
    catch( const std::exception& ex )
    {
        reply->set_ok(false);
        reply->set_msg(ex.what());
    }

    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::GetBackendInfo* msg)
{
    using namespace messages;
    switch( msg->req() )
    {
        case PEERS:
        {
            messages::PeerList* reply =
                    new messages::PeerList();
            m_backend->getPeers(reply);
            std::cout << "MessageHandler: queueing PeerList\n";
            m_outboundQueue->insert( new AutoMessage(reply) );
            break;
        }

        case KNOWN_PEERS:
        {
            messages::PeerList* reply =
                    new messages::PeerList();
            m_backend->getKnownPeers(reply);
            std::cout << "MessageHandler: queueing PeerList\n";
            m_outboundQueue->insert( new AutoMessage(reply) );
            break;
        }

        case MOUNT_POINTS:
        {
            messages::MountList* reply =
                    new messages::MountList();
            m_backend->getMounts(reply);
            std::cout << "MessageHandler: queueing MountList\n";
            m_outboundQueue->insert( new AutoMessage(reply) );
            break;
        }

        default:
        {
            messages::UserInterfaceReply* reply =
                    new messages::UserInterfaceReply();
            reply->set_ok(false);
            std::stringstream strm;
            strm << "Unrecognized get request " << msg->req();
            reply->set_msg( strm.str() );
            m_outboundQueue->insert( new AutoMessage(reply) );
            break;
        }
    }
}

void MessageHandler::handleMessage( messages::StartSync* msg )
{
    std::cout << "Handling start sync message\n";
    jobs::SendTree* job = new jobs::SendTree(m_backend,msg->peerid());
    m_backend->jobs()->enqueue(job);

    messages::SendTree* peerMsg = new messages::SendTree();
    peerMsg->set_dummy(5);
    m_backend->sendMessage(msg->peerid(),peerMsg,PRIO_NOW);

    messages::UserInterfaceReply* reply =
            new messages::UserInterfaceReply();
    reply->set_ok(true);
    std::stringstream strm;
    strm << "Sync not really implemented but OK";
    reply->set_msg( strm.str() );
    m_outboundQueue->insert( new AutoMessage(reply) );
}

void MessageHandler::handleMessage( messages::SendTree* msg )
{
    std::cout << "Handling send tree message\n";
    jobs::SendTree* job = new jobs::SendTree(m_backend,m_peerId);
    m_backend->jobs()->enqueue(job);
}

void MessageHandler::handleMessage( messages::Quit* msg )
{
    m_shouldQuit = true;
}

void MessageHandler::handleMessage( messages::Ping* msg )
{
    std::cout << "Handling PING\n";
    jobs::Pong* pong = new jobs::Pong(m_backend,m_peerId);
    m_backend->jobs()->enqueue(pong);
}

void MessageHandler::handleMessage( messages::Pong* msg )
{
    std::cout << "Handling PONG\n";
    jobs::Pong* pong = new jobs::Pong(m_backend,m_peerId);
    m_backend->jobs()->enqueue(pong);
}


void MessageHandler::handleMessage( messages::Subscribe* msg )
{

}


void MessageHandler::handleMessage( messages::Unsubscribe* msg )
{

}

void MessageHandler::handleMessage( messages::IdMap* msg )
{
    for( int i=0; i < msg->peermap_size(); i++ )
        m_backend->mapPeer( msg->peermap(i), m_peerMap );
    m_peerMap[0] = m_peerId;

    std::stringstream report;
    report << "MessageHandler: built id map: \n";


    for( PeerMap_t::iterator it = m_peerMap.begin();
            it != m_peerMap.end(); ++it )
    {
        report << boost::format("   %5d -> %-5d\n")
                    % it->first
                    % it->second;
    }
    std::cout << report.str();
}

void MessageHandler::handleMessage( messages::NodeInfo* msg )
{
    std::stringstream report;
    report << "MessageHandler: received node info: \n"
           << "    path: " << msg->parent() << "/" << msg->path() << "\n";
    std::cout << report.str();

    // first check to see if the file is currently checked out
    namespace fs = boost::filesystem;

    try
    {
        // decompose the path into a directory and file part
        fs::path root    = m_backend->realRoot();
        fs::path relpath = fs::path(msg->parent()) / msg->path();
        if( msg->parent() == "/" )
            relpath = "/" + msg->path();

        // check if we are subscribed
        if( !m_backend->db().isSubscribed(relpath) )
            ex()() << "Not subscribed to " << relpath;

        // create a version vector from the message
        VersionVector v_recv;
        for(int i=0; i < msg->version_size(); i++)
        {
            const messages::VersionEntry& entry = msg->version(i);
            v_recv[ entry.client() ] = entry.version();
        }

        // map version keys
        VersionVector v_theirs;
        mapVersion( v_recv, v_theirs );

        // assimilate version keys so that future file changes notify
        // connected peers
        m_backend->db().assimilateKeys( relpath, v_theirs );

        // retrieve my version
        VersionVector v_mine;
        m_backend->db().getVersion( relpath, v_mine );

        // compare version vectors, if their version is strictly newer then
        // we register it for download
        if( v_mine < v_theirs )
        {
            std::cout << "MessageHandler::(NodeInfo)  : "
                      << " version is strictly greater, adding download\n";
            m_backend->addDownload(m_peerId,relpath,msg->size(),v_theirs);

            messages::SendFile* sendFile = new messages::SendFile();
            sendFile->set_path(relpath.string());
            sendFile->set_tx(0);

            for( auto& pair : v_theirs )
            {
                messages::VersionEntry* entry = sendFile->add_version();
                entry->set_client(pair.first);
                entry->set_version(pair.second);
            }

            m_outboundQueue->insert( new AutoMessage(sendFile) );
        }
        else
        {
            std::cout << "MessageHandler::(NodeInfo)  : "
                      << " version " << v_theirs
                      << "is not strictly greater than " << v_mine << "\n";
        }
    }
    catch( const std::exception& ex )
    {
        std::cerr << "Failed to handle NodeInfo message: " << ex.what()
                  << "\n";
    }
}

void MessageHandler::handleMessage( messages::SendFile* msg )
{
    // read the version vector
    VersionVector v_recd;
    for(int i=0; i < msg->version_size(); i++)
    {
        auto& entry = msg->version(i);
        v_recd[ entry.client() ] = entry.version();
    }

    // map version keys
    VersionVector v_theirs;
    mapVersion(v_recd,v_theirs);

    std::stringstream report;
    report << "Received file tx request:"
           << "\n      path: " << msg->path()
           << "\n        tx: " << msg->tx()
           << "\n   version: " << v_theirs
           << "\n";

    std::cout << report.str();

    // create the job
    jobs::SendFile* sendFile = new jobs::SendFile(
            m_backend, m_peerId,
            msg->path(),
            msg->tx(),
            msg->offset(),
            v_theirs );

    // add the job
    m_backend->jobs()->enqueue(sendFile);
}


void MessageHandler::handleMessage( messages::NewVersion* msg )
{

}


void MessageHandler::handleMessage( messages::RequestFile* msg )
{

}


void MessageHandler::handleMessage( messages::FileChunk* msg )
{
    m_backend->mergeData(m_peerId,msg);
}



void MessageHandler::handleMessage( messages::DirChunk* msg )
{
    namespace fs = boost::filesystem;
    fs::path root = m_backend->realRoot();
    fs::path dir  = root / msg->path();
    try
    {
        if( !fs::exists(dir) )
            fs::create_directories(dir);
        m_backend->db().merge( msg );
    }
    catch( const std::exception& ex )
    {
        std::cerr << "MessageHandler::handleMessage() : WARNING "
                  << "failed to merge directory "
                  << msg->path()
                  << ", error: " << ex.what()
                  << "\n";
    }
}


void MessageHandler::handleMessage( messages::Invalid* msg )
{

}





} // namespace filesystem
} // namespace openbook

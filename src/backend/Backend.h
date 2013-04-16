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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/Backend.h
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_BACKEND_H_
#define OPENBOOK_FS_BACKEND_H_

#include <boost/filesystem.hpp>
#include <cpp-pthreads.h>

#include "Connection.h"
#include "FileDescriptor.h"
#include "LongJob.h"
#include "MessageHandler.h"
#include "NotifyPipe.h"
#include "SocketListener.h"
#include "MountPoint.h"




namespace   openbook {
namespace filesystem {

/// encapsulates and provides communication between backend components
class Backend
{
    public:
        typedef Pool<Connection>            ConnPool_t;
        typedef Pool<MessageHandler>        WorkerPool_t;
        typedef RefPtr<FileDescriptor>      FdPtr_t;
        typedef std::map<int,Connection*>   USPeerMap_t;
        typedef Synchronized<USPeerMap_t>   PeerMap_t;
        typedef std::vector<MountPoint*>    USMountMap_t;
        typedef Synchronized<USMountMap_t>  MountMap_t;
        typedef boost::filesystem::path     Path_t;

        enum Listeners
        {
            LISTEN_LOCAL,
            LISTEN_REMOTE,
            NUM_LISTENERS,
        };

    private:
        pthreads::Mutex  m_mutex;       ///< lock during callbacks
        std::string      m_configFile;  ///< configuration file to load
        NotifyPipe       m_termNote;    ///< globally signals termination
        std::string      m_displayName; ///< human readable string

        Path_t          m_dataDir;  ///< where data is stored
        Path_t          m_rootDir;  ///< base of the mirrored file system
        Path_t          m_dbFile;   ///< sqlite database with state
        std::string     m_pubKey;   ///< base64 encoded public key
        Path_t          m_privKey;  ///< path to private key file

        /// listens for incoming connections
        SocketListener   m_listeners[NUM_LISTENERS];

        /// threads for listeners
        pthreads::Thread m_listenThreads[NUM_LISTENERS];

        int             m_maxPeers;     ///< maximum number of peers
        ConnPool_t      m_connPool;     ///< connection pool
        WorkerPool_t    m_workerPool;   ///< worker pool

        pthreads::Thread    m_jobThread;    ///< for long jobs
        JobWorker           m_jobWorker;    ///< for long jobs

        PeerMap_t   m_peerMap;  ///< maps peer id to connection objects
        MountMap_t  m_mountPts; ///< stores mount points

        // for outgoing connections
        int         m_clientFamily; ///< address family AF_[INET|INET6|UNIX]
        std::string m_clientNode;   ///< interface to use (if a specific one)

    public:
        /// sets sensable defaults
        Backend();
        ~Backend();

        /// get the display name
        const std::string& displayName();

        /// get base64 encoded public key
        const std::string& publicKey();

        /// register and retrieve the peerId given the peers public key
        int registerPeer( const std::string& publicKey,
                          const std::string& displayName,
                          Connection* conn );

        /// unregister a connected peer
        void unregisterPeer( int peerId );

        /// return the path to the private key file
        std::string privateKeyFile();

        /// callback for new peer connections
        void onConnect(FdPtr_t sockfd, bool remote);

        /// add a mount point
        void mount( const std::string& mountPoint, int argc, char** argv );

        /// remote a mount point by id
        void unmount( int id );

        /// send a message to a specific peer
        template <typename Message_t>
        void sendMessage( int peerId, Message_t* msg )
        {
            USPeerMap_t::iterator it = m_peerMap.lockFor()->find(peerId);
            if( it == m_peerMap.subvert()->end() )
            {
                std::cout << "Backend: not sending "
                          << messageIdToString( MessageTypeToId<Message_t>::ID )
                          << " message b/c " << peerId
                          << " isn't in map \n";
                return;
            }

            it->second->enqueueMessage(peerId,msg);
        }



        /// return a pointer to the job worker so that handlers can queue
        /// long jobs
        JobWorker* jobs(){ return &m_jobWorker; }

        /// return the the data directory of the backend
        const Path_t dataDir(){ return m_dataDir; }

        /// return the real root of the filesystem
        const Path_t realRoot(){ return m_rootDir; }



        // -------- CONFIG/UI OPS -----------
        // each of these can be called as the result of
        //  1. loading a configuration file
        //  2. a gui message
        //  3. a command line message

        /// set the display name for this replica
        void setDisplayName( const std::string& name );

        /// set the data directory, where actual file storage is
        void setDataDir( const std::string& dir );

        /// set the local socket where we listen for local connections... i.e.
        /// command line client or gui client
        void setLocalSocket( int port );

        /// set th remote socket where we listen for remote connections... i.e.
        /// replicas
        void setRemoteSocket( int addressFamily,
                                const std::string& node,
                                const std::string& service );

        /// set the socket we use connecting to peers
        void setClientSocket( int addressFamily, const std::string& node );

        /// set the size of the connection pool
        void setMaxConnections( int maxConnections );

        /// loads a configuration file
        void loadConfig(const std::string& filename);

        /// saves a configuration file
        void saveConfig(const std::string& filename);

        /// attempt to connect to a peer
        void attemptConnection( bool isRemote,
                                const std::string& node,
                                const std::string& service );

    private:
        /// parses the command line
        void parse( int argc, char** argv );

    public:
        /// call from main()
        int run(int argc, char** argv);
};


} //< namespace filesystem
} //< namespace openbook















#endif // BACKEND_H_

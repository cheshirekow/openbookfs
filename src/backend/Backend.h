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
#include "MessageHandler.h"
#include "NotifyPipe.h"
#include "SocketListener.h"



namespace   openbook {
namespace filesystem {

/// encapsulates and provides communication between backend components
class Backend
{
    public:
        typedef Pool<Connection>        ConnPool_t;
        typedef Pool<MessageHandler>    WorkerPool_t;
        typedef boost::filesystem::path Path_t;

        enum Listeners
        {
            LISTEN_LOCAL,
            LISTEN_REMOTE,
            NUM_LISTENERS,
        };

    private:
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




    public:
        /// sets sensable defaults
        Backend();
        ~Backend();

        /// get base64 encoded public key
        const std::string& publicKey();

        /// register and retrieve the peerId given the peers public key
        int connectPeer( const std::string& publicKey );

        /// unregister a connected peer
        void disconnectPeer( int peerId );

        /// return the path to the private key file
        std::string privateKeyFile();

        /// callback for new peer connections
        void onConnect(int sockfd, bool remote);

        // -------- CONFIG OPS -----------
        // each of these can be called as the result of
        //  1. loading a configuration file
        //  2. a gui message
        //  3. a command line message
        void setDisplayName( const std::string& name );

        void setDataDir( const std::string& dir );

        void setLocalSocket( int port );

        void setRemoteSocket( int addressFamily,
                                const std::string& node,
                                const std::string& service );

        void setMaxConnections( int maxConnections );

    private:
        /// parses the command line
        void parse( int argc, char** argv );

        /// loads a configuration file
        void loadConfig(const std::string& filename);

        /// saves a configuration file
        void saveConfig(const std::string& filename);

    public:
        /// call from main()
        int run(int argc, char** argv);
};

}
}















#endif // BACKEND_H_

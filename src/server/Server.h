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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Server.h
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_SERVER_H_
#define OPENBOOK_SERVER_H_

#include <cpp-pthreads.h>

#include <crypto++/rsa.h>
#include <cryptopp/osrng.h>

#include <bitset>

namespace   openbook {
namespace filesystem {
 namespace    server {


/// encapsulates details about the server and it's configuration
struct Server
{
    public:
        enum AuthOpts
        {
            AUTH_PASSWORD=0,
            AUTH_VOUCH_FOR,
            NUM_AUTH
        };

        typedef std::bitset<NUM_AUTH> AuthOpt;

    private:
        pthreads::Mutex m_mutex;    ///< locks this data

        std::string  m_dataDir;      ///< server data location
        std::string  m_rootDir;      ///< file system root
        std::string  m_pubKeyFile;   ///< public key file
        std::string  m_privKeyFile;  ///< private key file
        std::string  m_dbFile;       ///< database file

        AuthOpt      m_auth;     ///< set of authorization methods we'll allow
        std::string  m_password; ///< for authorizing clients
        std::string  m_addressFamily;   ///< address family of iface
        std::string  m_iface;           ///< which interface to bind
        std::string  m_port;        ///< which port to listen on
        int          m_maxConn;     ///< number of connections to accept
        int          m_maxWorkers;  ///< number of worker threads to run

    public:
        Server();
        ~Server();

        /// load a configuration from a file (will throw an exception on
        /// errors)
        void initConfig( const std::string& configFile );

        const std::string& dataDir()        const{ return m_dataDir; }
        const std::string& rootDir()        const{ return m_rootDir; }
        const std::string& pubKeyFile()     const{ return m_pubKeyFile;    }
        const std::string& privKeyFile()    const{ return m_privKeyFile;   }
        const std::string& password()       const{ return m_password;      }
        const std::string& addressFamily()  const{ return m_addressFamily; }
        const std::string& iface()          const{ return m_iface;         }
        const std::string& port()           const{ return m_port;          }
        const std::string& dbFile()         const{ return m_dbFile;        }

        int maxConn()    const { return m_maxConn;    }
        int maxWorkers() const { return m_maxWorkers; }

};




} // namespace server 
} // namespace filesystem
} // namespace openbook



#endif // SERVER_H_

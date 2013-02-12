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

#include <boost/filesystem.hpp>


namespace   openbook {
namespace filesystem {


/// encapsulates details about the server and it's configuration
struct Server
{
        pthreads::Mutex m_mutex;    ///< locks this data

        boost::filesystem::path m_dataDir;      ///< server data location
        boost::filesystem::path m_rootDir;      ///< file system root
        boost::filesystem::path m_pubKeyFile;   ///< public key file
        boost::filesystem::path m_privKeyFile;  ///< private key file

        std::string  m_salt      ///< for authorizing clients
        std::string  m_password; ///< for authorizing clients
        std::string  m_iface;    ///< which interface to bind
        int          m_port;     ///< which port to listen on
        int          m_maxConn;  ///< number of connections to accept


    public:
        Server();
        ~Server();

        void initData( const std::string& dataDir );
        void initConfig( const std::string& configFile );
};




} // namespace filesystem
} // namespace openbook



#endif // SERVER_H_

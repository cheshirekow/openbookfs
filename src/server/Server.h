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

#include <crypto++/files.h>
#include <crypto++/rsa.h>
#include <crypto++/rng.h>

#include <boost/filesystem.hpp>


namespace   openbook {
namespace filesystem {


/// encapsulates details about the server
class Server
{
    private:
        pthreads::Mutex             m_mutex;
        boost::filesystem::path     m_dataDir;
        CryptoPP::RSA::PublicKey    m_pubKey;
        CryptoPP::RSA::PrivateKey   m_privKey;

    public:
        Server();
        ~Server();

        void initData( const std::string& dataDir );
        void initKeys( const std::string& pubKey,
                       const std::string& privKey );


};




} // namespace filesystem
} // namespace openbook



#endif // SERVER_H_

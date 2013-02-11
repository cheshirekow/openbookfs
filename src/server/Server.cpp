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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Server.cpp
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "Server.h"
#include "ExceptionStream.h"
#include <iostream>
#include <fstream>
#include <string>

#include <crypto++/files.h>
#include <crypto++/cryptlib.h>


namespace   openbook {
namespace filesystem {


Server::Server()
{
    m_mutex.init();
}

Server::~Server()
{
    m_mutex.destroy();
}

void Server::initData(const std::string& dataDir)
{
    namespace fs = boost::filesystem;

    // check that the data directory and subdirectories exist
    if( !fs::exists( fs::path(dataDir) ) )
    {
        std::cout << "creating data directory: "
                  << fs::absolute( fs::path(dataDir) )
                  << std::endl;
        bool result = fs::create_directories( fs::path(dataDir ) );
        if( !result )
            ex()() << "failed to create data directory: " << dataDir;
    }

    if( !fs::exists( fs::path(dataDir) / "client_keys" ) )
    {
        std::cout << "creating directory : "
                  << fs::absolute( fs::path(dataDir) / "client_keys" )
                  << std::endl;
        bool result = fs::create_directories( fs::path(dataDir) / "client_keys" );
        if( !result )
            ex()() << "failed to create data directory: "
                      << dataDir + "/client_keys";
    }
}

void Server::initKeys(const std::string& pubKey, const std::string& privKey)
{
    try
    {
        // open a stream to the public key file
        std::ifstream in(pubKey.c_str(), std::ios::in | std::ios::binary);
        if (!in)
            ex()() << "Failed to open " << pubKey << " for reading ";

        // seek to the end of the file to get it's size
        in.seekg(0, std::ios::end);

        // resize the storage space
        m_pubStr.resize((unsigned int)in.tellg(),'\0');

        // seek back to the beginning
        in.seekg(0, std::ios::beg);

        // read in the entire file
        in.read(&m_pubStr[0], m_pubStr.size());

        // seek back to the beginning again
        in.seekg(0, std::ios::beg);

        // read into public key
        CryptoPP::FileSource keyFile(in,true);
        CryptoPP::ByteQueue  queue;
        keyFile.TransferTo(queue);
        queue.MessageEnd();

        m_pubKey.Load(queue);

        // close the file
        in.close();
    }
    catch( CryptoPP::Exception& cex )
    {
        ex()() << "Failed to load public key from " << pubKey
               <<  " : " << cex.what();
    }

    try
    {
        CryptoPP::FileSource keyFile(privKey.c_str(),true);
        CryptoPP::ByteQueue  queue;
        keyFile.TransferTo(queue);
        queue.MessageEnd();

        m_privKey.Load(queue);
    }
    catch( CryptoPP::Exception& cex )
    {
        ex()() << "Failed to load private key from " << privKey
               <<  " : " << cex.what();
    }

    CryptoPP::LC_RNG rng(0x01);
    if( !m_pubKey.Validate(rng,3) )
        ex()() << "Failed to validate public key";

    if( !m_privKey.Validate(rng,3) )
        ex()() << "Failed to validate private key";
}


void Server::decrypt( const std::string& cipher,
                      std::string& plain )
{
    using namespace CryptoPP;
    RSAES_OAEP_SHA_Decryptor d(m_privKey);

    StringSource ss2(cipher, true,
            new PK_DecryptorFilter(m_rng, d, new StringSink(plain)
   ) // PK_DecryptorFilter
); // StringSource
}



} // namespace filesystem
} // namespace openbook

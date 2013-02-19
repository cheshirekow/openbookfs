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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/Client.cpp
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */



#include "Client.h"
#include "ExceptionStream.h"
#include <iostream>
#include <fstream>
#include <string>

#include <yaml-cpp/yaml.h>
#include <crypto++/osrng.h>
#include <crypto++/rsa.h>
#include <crypto++/filters.h>
#include <crypto++/files.h>

#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>


namespace   openbook {
namespace filesystem {


Client::Client()
{
    m_nextId = 0;
    m_mutex.init();
}

Client::~Client()
{
    m_mutex.destroy();
}


void Client::initConfig(const std::string& configFile)
{
    namespace fs = boost::filesystem;

    // verify that the config file exists
    if( !fs::exists( fs::path(configFile) ) )
        ex()() << "Client configuration file not found: " << configFile;

    std::ifstream in(configFile.c_str());
    if(!in)
        ex()() << "Failed to open " << configFile << " for reading";

    YAML::Parser parser(in);
    YAML::Node   config;
    parser.GetNextDocument(config);

    std::string serverStr;

    config["password"]      >> m_password;
    config["dataDir"]       >> m_dataDir;
    config["rootDir"]       >> m_rootDir;
    config["addressFamily"] >> m_addressFamily;
    config["iface"]         >> m_iface;
    config["maxWorkers"]    >> m_maxWorkers;
    config["server"]        >> m_server;

    namespace fs = boost::filesystem;

    m_realRoot = fs::path(m_dataDir) / "real_root";

    // check that the data directory and subdirectories exist
    if( !fs::exists( m_realRoot  ) )
    {
        std::cout << "creating data directory: "
                  << fs::absolute( m_realRoot  )
                  << std::endl;
        bool result = fs::create_directories( m_realRoot  );
        if( !result )
            ex()() << "failed to create data directory: " << m_realRoot ;
    }

    // if there is no private key file then create one
    fs::path privKeyFile = fs::path(m_dataDir) / "id_rsa.der";
    fs::path pubKeyFile  = fs::path(m_dataDir) / "id_rsa_pub.der";

    if( !fs::exists( privKeyFile) || !fs::exists (pubKeyFile) )
    {
        std::cout << "No public or private keyfile in "
                     "data directory, generating now\n";

        using namespace CryptoPP;
        AutoSeededRandomPool rng;
        RSA::PrivateKey rsaPrivate;
        rsaPrivate.GenerateRandomWithKeySize(rng, 3072 );
        RSA::PublicKey  rsaPublic(rsaPrivate);

        ByteQueue queue;

        // save private key
        rsaPrivate.Save(queue);
        FileSink privKeySink( privKeyFile.string().c_str() );
        queue.CopyTo(privKeySink);
        privKeySink.MessageEnd();
        queue.Clear();

        // save public key
        rsaPublic.Save(queue);
        FileSink pubKeySink( pubKeyFile.string().c_str() );
        queue.CopyTo( pubKeySink );
        pubKeySink.MessageEnd();
        queue.Clear();
    }

    m_pubKeyFile    = pubKeyFile.string();
    m_privKeyFile   = privKeyFile.string();

    // initialize the message database
    using namespace soci;

    std::cout << "Initializing database" << std::endl;
    m_dbFile = fs::path(m_dataDir) / "store.sqlite";
    session sql(sqlite3,m_dbFile.string());

    sql << "CREATE TABLE IF NOT EXISTS conflict_files ("
            "conflict_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "path TEXT NOT NULL ) ";

    sql << "CREATE TABLE IF NOT EXISTS downloads ("
            "tx_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "path TEXT NOT NULL ) ";

    sql << "CREATE TABLE IF NOT EXISTS current_messages ("
            "msg_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "msg_type INTEGER NOT NULL, "
            "msg BLOB)";

    sql << "CREATE TABLE IF NOT EXISTS old_messages ("
            "msg_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "msg_type INTEGER NOT NULL, "
            "msg BLOB)";
}


uint64_t Client::nextId()
{
    pthreads::ScopedLock lock(m_mutex);
    return ++m_nextId;
}




} // namespace filesystem
} // namespace openbook

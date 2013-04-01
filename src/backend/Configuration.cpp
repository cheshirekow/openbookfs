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
 *  @file   src/client_fs/Configuration.cpp
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */



#include "Configuration.h"
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

Configuration::Configuration()
{
    // initialize the kv-store with sensable defaults
    static_cast<KVStore_t&>(*this)
    ( "displayName"  , std::string("Unnamed")  )
    ( "password"     , std::string("fabulous") )
    ( "dataDir"      , std::string("./data")   )
    ( "rootDir"      , std::string("./root")   )
    ( "addressFamily", std::string("AF_INET")  )
    ( "iface"        , std::string("any")      )
    ( "maxWorkers"   , (int)20                 );

    m_mutex.init();
}

Configuration::~Configuration()
{
    m_mutex.destroy();
}


void initializeDataDir()
{

}

void Configuration::loadConfig()
{
    loadConfig(m_configFile);
}

void Configuration::loadConfig(const std::string& configFile)
{
    namespace fs = boost::filesystem;

    // load the configuration into the kv-store
    KVStore_t::read(configFile);

    path dataDir = getPath("dataDir");
    this->set( "realRoot",   ( dataDir / "real_root"   ).string() );
    this->set( "dbFile",     ( dataDir / "store.sqlite").string() );
    this->set( "privKeyFile",( dataDir / "id_rsa.der"  ).string() );
    this->set( "pubKeyFile", ( dataDir / "pub_rsa.der" ).string() );



    // check that the data directory and subdirectories exist
    path realRoot = getPath("realRoot");
    if( !fs::exists( realRoot  ) )
    {
        std::cout << "creating data directory: "
                  << fs::absolute( realRoot  )
                  << std::endl;
        bool result = fs::create_directories( realRoot  );
        if( !result )
            ex()() << "failed to create data directory: " << realRoot ;
    }

    // if there is no private key file then create one
    path privKeyFile = getPath("privKeyFile");
    path pubKeyFile  = getPath("pubKeyFile");

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

    // initialize the message database
    using namespace soci;

    std::cout << "Initializing database" << std::endl;
    path dbFile = getPath("dbFile");
    session sql(sqlite3,dbFile.string());

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
    sql.close();

    // initialize the root directory if not already initialized
    std::string rootMeta = ( realRoot / "obfs.sqlite").string();
    sql.open(sqlite3,rootMeta);

    sql << "CREATE TABLE IF NOT EXISTS meta ("
            "key TEXT UNIQUE NOT NULL, "
            "value)";

    // note: states are
    // 0: synced
    // 1: dirty
    // 2: stale
    // 3: conflict
    sql << "INSERT OR IGNORE INTO meta (key,value) values ('state',0)";

    sql << "CREATE TABLE IF NOT EXISTS version ("
            "client TEXT UNIQUE NOT NULL, "
            "version INTEGER NOT NULL) ";

    // note: file types are
    // 1: regular file
    // 2: subdirectory
    // 3: symbolic link
    // 4: hard link (not yet supported)
    sql << "CREATE TABLE IF NOT EXISTS entries ("
            "path VARCHAR(255) UNIQUE NOT NULL, "
            "type INTEGER NOT NULL DEFAULT(1), "
            "subscribed INTEGER NOT NULL, "
            "size INTEGER NOT NULL, "
            "ctime INTEGER NOT NULL, "
            "mtime INTEGER NOT NULL) ";
}

void Configuration::saveConfig()
{
    saveConfig(m_configFile);
}

void Configuration::saveConfig( const std::string& configFile )
{
    KVStore_t::write(configFile);
}

boost::filesystem::path Configuration::getPath( const std::string& key )
{
    return get<std::string>(key);
}


} // namespace filesystem
} // namespace openbook

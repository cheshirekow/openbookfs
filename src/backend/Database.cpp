/**
 *  @file   /openbook_fs/[Source directory]/src/backend/Database.cpp
 *
 *  @date   Apr 19, 2013
 *  @author Josh Bialkowski <jbialk@mit.edu>
 *  @brief  
 */



#include "Database.h"
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>


namespace   openbook {
namespace filesystem {

Database::Database()
{
    m_mutex.init();
}

Database::~Database()
{
    m_mutex.destroy();
}

void Database::setPath( const Path_t& path )
{
    m_dbFile = path;
}

void Database::init()
{
    pthreads::ScopedLock lock(m_mutex);

    // initialize the message database
    using namespace soci;

    std::cout << "Initializing database" << std::endl;
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

    sql << "CREATE TABLE IF NOT EXISTS known_clients ("
            "client_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "client_key TEXT NOT NULL UNIQUE, "
            "client_name TEXT NOT NULL) ";


}

void Database::getClientMap( LockedPtr<USIdMap_t>& map )
{
    pthreads::ScopedLock lock(m_mutex);

    // initialize the message database
    using namespace soci;

    session sql(sqlite3,m_dbFile.string());

    // initialize the id map
    std::string base64Key;
    int         peerId;
    typedef soci::rowset<soci::row>    rowset;
    typedef std::pair<std::string,int> mapentry;
    rowset rs =
            ( sql.prepare << "select client_id,client_key FROM known_clients");
    for( auto& row : rs )
        map->insert(
                mapentry(row.get<std::string>(1) ,row.get<int>(0) ) );
}

int Database::registerPeer( const std::string&  base64,
                             const std::string& displayName )
{
     pthreads::ScopedLock lock(m_mutex);

    // initialize the message database
    using namespace soci;

    // if public key is empty string then that means this is a GUI only
    // connection so we dont bother putting it in the map
    std::cout << "Database::registerPeer: "
                 "Putting base64 client key into db:\n"
              << base64 << std::endl;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    // insert the key into the database if it isn't already there
    sql << "INSERT OR IGNORE INTO known_clients (client_key, client_name) "
           "VALUES ('"<< base64 << "','" << displayName << "')";

    // now select out the id
    int peerId = 0;
    sql << "SELECT client_id FROM known_clients WHERE client_key='"
        << base64 << "'",
            soci::into(peerId);

    // update the client name
    sql << "UPDATE known_clients SET client_name='" << displayName
        << "' WHERE client_id=" << peerId;

    return peerId;
}


} //< namespace filesystem
} //< namespace openbook




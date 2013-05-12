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
#include <boost/format.hpp>
#include "ExceptionStream.h"


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

    // stores a list of files that we are in the progress of receiving, these
    // are either newer files or conflict files
    sql << "CREATE TABLE IF NOT EXISTS downloads ("
            // the target of the file being downloaded
            "path   TEXT NOT NULL, "
            // the peer we're downloading from
            "peer   INTEGER NOT NULL, "
            // transaction id, incremented if download is pre-empted
            "tx     INTEGER NOT NULL, "
            // the temporary file we're storing data in
            "temp   TEXT NOT NULL, "
            // the number of bytes we have received
            "recd   INTEGER NOT NULL, "
            // the size of the file in bytes
            "size   INTEGER NOT NULL,"
            // a path/peer is unique
            "PRIMARY KEY(path,peer) ) ";

    // stores version vectors for conflict files
    sql << "CREATE TABLE IF NOT EXISTS downloads_v ("
            // the file in conflict
            "path       TEXT NOT NULL,"
            // the peer who has a conflicting version
            "peer       INTEGER NOT NULL,"
            // the key of the version vector
            "v_peer     INTEGER NOT NULL, "
            // the value of the version vector
            "v_version  INTEGER NOT NULL,"
            // a path/peer is unique
            "PRIMARY KEY(path,peer,v_peer) ) ";



    // stores a list of files in conflict
    sql << "CREATE TABLE IF NOT EXISTS conflicts ("
            // the file in conflict
            "path   TEXT NOT NULL,"
            // the peer who has a conflicting version
            "peer   INTEGER NOT NULL,"
            // the location of the downloaded copy of the conflicting
            // version
            "stage  TEXT NOT NULL,"
            // size of the peers version of the file in bytes
            "size   INTEGER NOT NULL,"
            // number of bytes that we have received
            "recv   INTEGER NOT NULL,"
            // a path/peer is unique
            "PRIMARY KEY(path,peer) ) ";

    // stores version vectors for conflict files
    sql << "CREATE TABLE IF NOT EXISTS conflict_v ("
            // the file in conflict
            "path       TEXT NOT NULL,"
            // the peer who has a conflicting version
            "peer       INTEGER NOT NULL,"
            // the key of the version vector
            "v_peer     INTEGER NOT NULL, "
            // the value of the version vector
            "v_version  INTEGER NOT NULL,"
            // a path/peer is unique
            "PRIMARY KEY(path,peer,v_peer) ) ";

    // stores clients that we know about and assigns them a unique
    // numerical index
    sql << "CREATE TABLE IF NOT EXISTS known_clients ("
            // unique numerical identifier for the client
            "client_id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            // base64 encoded public key
            "client_key  TEXT NOT NULL UNIQUE, "
            // human readable name for this machine
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

    try
    {
        rowset rs =
                ( sql.prepare << "select client_id,client_key FROM known_clients");
        for( auto& row : rs )
            map->insert(
                    mapentry(row.get<std::string>(1) ,row.get<int>(0) ) );
    }
    catch( const std::exception& ex )
    {
        std::cerr << "Database: Failed to retrieve client map: " << ex.what()
                  << "\n";
    }
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

    try
    {
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
    catch( const std::exception& ex )
    {
        std::cerr << "Database: Failed to register peer: " << ex.what()
                  << "\n";
    }

    return 0;
}


void Database::buildPeerMap( messages::IdMap* map )
{
    pthreads::ScopedLock lock(m_mutex);

    // initialize the message database
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    // initialize the id map
    typedef soci::rowset<soci::row>    rowset;

    try
    {
        rowset rs = ( sql.prepare << "SELECT * FROM known_clients");
        for( auto& row : rs )
        {
            messages::IdMapEntry* entry = map->add_peermap();
            entry->set_peerid     ( row.get<int>(0)         );
            entry->set_publickey  ( row.get<std::string>(1) );
            entry->set_displayname( row.get<std::string>(2) );
        }
    }
    catch( const std::exception& ex )
    {
        std::cerr << "Database: Failed to build client map: " << ex.what()
                  << "\n";
    }
}


void Database::addDownload( int64_t peer,
                            const Path_t& path,
                            int64_t size,
                            const VersionVector& version,
                            const Path_t& stageDir  )
{
    pthreads::ScopedLock lock(m_mutex);

    namespace fs = boost::filesystem;
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    // initialize the id map
    try
    {
        // get previous instance of the download
        int count=0;
        sql << boost::format(
                "SELECT count(*) FROM downloads WHERE path='%s' AND peer=%d" )
                % path.string()
                % peer, into(count);

        // if the download exists then get the version being downloaded
        if(count > 0)
        {
            std::cout << "Database::addDownload() : download already exists, "
                        "checking if I need to update\n";

            // perform the version query
            rowset<row> rs = ( sql.prepare << boost::format(
                    "SELECT v_peer,v_version FROM downloads_v "
                    " WHERE path='%s' AND peer='%d'" )
                    % path.string()
                    % peer );

            // build the version vector
            VersionVector v_prev;
            for( auto& row : rs )
                v_prev[ row.get<int64_t>(0) ] = row.get<int64_t>(1);

            // if the current download is the not less then the requested
            // download there is nothing left to do
            if( v_prev >= version )
            {
                std::cerr << "Database::addDownload ignoring download for "
                          << peer << " : "
                          << path << "because already in progress\n";
                return;
            }

            // otherwise simply reset the bytes received, increment the
            // transaction number, and set the size
            sql << boost::format(
                    "UPDATE downloads SET tx=tx+1, recd=0, size=%d"
                    " WHERE path='%s' AND peer='%d'" )
                    % size
                    % path.string()
                    % peer;

            // delete any old version info
            sql << boost::format(
                    "DELETE FROM downloads_v WHERE path='%s' AND peer=%d")
                    % path.string()
                    % peer;

            // insert new version info
            for( auto& pair : version )
            {
                sql << boost::format(
                    "INSERT INTO downloads_v (path,peer,v_peer,v_version) "
                    "VALUES ('%s',%d,%d,%d)" )
                        % path.string()
                        % peer
                        % pair.first
                        % pair.second
                    ;
            }

            // truncate the temporary file
            std::string temp;
            sql << boost::format(
                "SELECT temp FROM downloads WHERE path='%s' AND peer=%d" )
                % path.string()
                % peer, into(temp);

            truncate( (stageDir / temp).c_str(), size );

            return;
        }

        std::cout << "Database::addDownload() : creating new download\n";

        // if the file is not already being downloaded
        // create a file to store the download
        std::string tpl = ( stageDir / "XXXXXX").string();
        int fd = mkstemp( &tpl[0] );
        if( fd < 0 )
        {
            codedExcept(errno)()
                << "Failed to create a temporary with template " << tpl;
        }
        ftruncate(fd,size);
        close(fd);

        std::string temp = tpl.substr(tpl.size()-6,6);

        // insert the download
        sql << boost::format(
                "INSERT INTO downloads (path,peer,tx,temp,recd,size) "
                "VALUES ('%s',%d,0,'%s',0,%d)" )
                % path.string()
                % peer
                % temp
                % size;

        // insert new version info
        for( auto& pair : version )
        {
            sql << boost::format(
                "INSERT INTO downloads_v (path,peer,v_peer,v_version) "
                "VALUES ('%s',%d,%d,%d)" )
                    % path.string()
                    % peer
                    % pair.first
                    % pair.second
                ;
        }
    }
    catch( const std::exception& ex )
    {
        std::cerr << "Database::addDownload Failed to add download: "
                  << ex.what()
                  << "\n";
    }
}


} //< namespace filesystem
} //< namespace openbook




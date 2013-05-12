/**
 *  @file   /openbook_fs/[Source directory]/src/backend/Database.cpp
 *
 *  @date   Apr 19, 2013
 *  @author Josh Bialkowski <jbialk@mit.edu>
 *  @brief  
 */

#include <fcntl.h>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <boost/format.hpp>

#include "Database.h"
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

    // stores a list of all files that we know about
    sql << "CREATE TABLE IF NOT EXISTS files ("
            // unique identifier
            "id     INTEGER PRIMARY KEY AUTOINCREMENT, "
            // link to parent
            "parent INTEGER NOT NULL, "
            // name of the entry
            "node   TEXT NOT NULL, "
            // full path of the entry
            "path   TEXT UNIQUE NOT NULL, "
            // whether or not it's checked out
            "subscribed INTEGER NOT NULL, "
            // parent, node pairs must be unique
            "UNIQUE (parent,node)"
            ") ";

    sql << "INSERT OR IGNORE INTO files (parent,node,path,subscribed) "
            " VALUES(0,'/','/',0) ";

    // stores local version information for each checked out file
    sql << "CREATE TABLE IF NOT EXISTS version ("
            // the file it belongs to
            "file_id    INTEGER NOT NULL, "
            // version key
            "peer       INTEGER NOT NULL, "
            // verstion value
            "version    INTEGER NOT NULL, "
            // file_id,peer pairs must be unique
            "UNIQUE (file_id,peer)"
            ") ";

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



void Database::mergeData( int64_t peer,
                            const Path_t& stageDir,
                            const Path_t& rootDir,
                            messages::FileChunk* chunk )
{
    pthreads::ScopedLock lock(m_mutex);

    namespace fs = boost::filesystem;
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    Path_t relpath = chunk->path();

    // initialize the id map
    try
    {
        std::string temp;
        int64_t     tx;
        int64_t     recd;
        int64_t     size;

        // get download context
        sql << boost::format(
                "SELECT temp,tx,recd,size FROM downloads "
                    "WHERE path='%s' AND peer=%d" )
                % chunk->path()
                % peer,
                into(temp),
                into(tx),
                into(recd),
                into(size);

        if( tx > chunk->tx() )
        {
            std::stringstream report;
            report << "Database::mergeData : aborting merge b/c file chunk"
                        " is from an older version\n";
            std::cout << report;
        }

        Path_t fullpath = stageDir / temp;

        // otherwise read in some bytes
        int fd = open(fullpath.c_str(), O_WRONLY );
        if( fd < 0 )
        {
            codedExcept(errno)() << "Database::mergeData: Failed to open "
                               << fullpath;
        }

        // move the read head
        if( chunk->offset() != lseek(fd,chunk->offset(),SEEK_SET) )
        {
            close(fd);
            codedExcept(errno)() << "Database::mergeData: Failed to seek to offset "
                               << chunk->offset()
                               << " of file " << fullpath;
        }

        // write the data
        int bytesWritten = write(fd,&chunk->data()[0],chunk->data().size());
        if( bytesWritten < 0 )
        {
            close(fd);
            codedExcept(errno)() << "Database::mergeData: Failed to write to "
                               << fullpath;
        }

        // close the file
        close(fd);

        // update bytes written
        recd += chunk->data().size();
        sql << boost::format(
            "UPDATE downloads SET recd=%d "
                "WHERE path='%s' AND peer=%d" )
            % recd
            % chunk->path()
            % peer;

        if( recd >= size )
        {
            // check to make sure that this file is truly newer
            VersionVector v_mine;
            getVersion( relpath, v_mine );

            // perform the version query
            rowset<row> rs = ( sql.prepare << boost::format(
                    "SELECT v_peer,v_version FROM downloads_v "
                    " WHERE path='%s' AND peer=%d" )
                    % relpath.string()
                    % peer );

            std::cout << "Database::mergeData : building version vector\n";

            // build the version vector
            VersionVector v_theirs;
            for( auto& row : rs )
                v_theirs[ row.get<int>(0) ] = row.get<int>(1);

            std::cout << "Database::mergeData : version built\n";

            // if it is truely newer, move the file and delete the
            // download
            if( v_mine < v_theirs )
            {
                int result = rename( (stageDir/temp).c_str(),
                                        (rootDir/relpath).c_str() );
                if( result < 0 )
                {
                    codedExcept(errno)() << "Database::mergeData : "
                            "Failed to rename " << (stageDir/temp)
                            << " -> " << (rootDir/relpath);
                }

                // update the version vector
                setVersion( relpath, v_theirs );

                // delete the download if it is complete
                sql << boost::format(
                    "DELETE FROM downloads WHERE path='%s' AND peer=%d" )
                    % chunk->path()
                    % peer;

                sql << boost::format(
                    "DELETE FROM downloads_v WHERE path='%s' AND peer=%d" )
                    % chunk->path()
                    % peer;
            }
        }
    }
    catch( const std::exception& ex )
    {
        std::cerr << "Database::mergeData failed: "
                  << ex.what()
                  << "\n";
    }
}



void Database::lockless_mknod( const Path_t& path )
{
    namespace fs = boost::filesystem;

    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        int64_t parentId = -1;
        // get the parent path
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.parent_path().string(),
                soci::into(parentId);
        if( parentId < 0 )
        {
            ex()() << "parent directory " << path.parent_path()
                   << "does not exist";
        }

        // create the node
        sql << boost::format(
                "INSERT OR IGNORE INTO files (parent,node,path,subscribed) "
                "VALUES (%d,'%s','%s',1)")
                % parentId
                % path.filename().string()
                % path.string();

        int64_t fileId = -1;
        sql << boost::format(
                "SELECT id FROM files where parent=%d AND node='%s'")
                % parentId
                % path.filename().string(),
                soci::into(fileId);

        if( fileId < 0 )
            ex()() << "file was not entered into database";

        // set the initial version of the file
        sql << boost::format(
                "INSERT OR IGNORE INTO version (file_id,peer,version) "
                "VALUES(%d,0,0)" ) % fileId;
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::mknod('" << path << "') failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_unlink( const Path_t& path )
{
    namespace fs = boost::filesystem;

    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        // get the fileId
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        sql << boost::format("UPDATE files SET subscribed=0 WHERE id=%d")
                % fileId;
        sql << boost::format("DELETE FROM version WHERE file_id=%d")
                % fileId;
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::unlink('" << path << "') failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_readdir( const Path_t& path,
                void *buf, fuse_fill_dir_t filler, off_t offset )
{
    namespace fs = boost::filesystem;

    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        // get the fileId
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        // now iterate over all children
        typedef soci::rowset<std::string> rowset_t;
        rowset_t rs = (
            sql.prepare << boost::format(
                "SELECT node FROM files WHERE parent=%d ORDER BY node LIMIT -1 OFFSET %d" )
                % fileId
                % offset );

        for( auto& path : rs )
        {
            if( filler(buf,path.c_str(),NULL,++offset) )
                return;
        }
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::readdir('" << path << "', fuse) failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_readdir( const Path_t& path,
                messages::DirChunk* msg )
{
    namespace fs = boost::filesystem;

    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        // get the fileId
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        // now iterate over all children
        typedef soci::rowset<std::string> rowset_t;
        rowset_t rs = (
            sql.prepare << boost::format(
                "SELECT node FROM files WHERE parent=%d ORDER BY node" )
                % fileId );

        for( auto& path : rs )
        {
            messages::DirEntry* entry = msg->add_entries();
            entry->set_path(path);
        }
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::readdir('" << path << "', msg) failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_readdir( const Path_t& path,
                std::list<std::string>& listing,
                bool subscribed )
{
    namespace fs = boost::filesystem;

    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        // get the fileId
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        std::stringstream query;
        if(subscribed)
        {
            query << boost::format(
                    "SELECT node FROM files "
                    "WHERE parent=%d AND subscribed=1 "
                    "ORDER BY node" )
                    % fileId;
        }
        else
        {
            query << boost::format(
                    "SELECT node FROM files "
                    "WHERE parent=%d "
                    "ORDER BY node" )
                    % fileId;
        }


        // now iterate over all children
        typedef soci::rowset<std::string> rowset_t;
        rowset_t rs = ( sql.prepare << query.str() );

        for( auto& path : rs )
            listing.push_back(path);
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::readdir('" << path << "', msg) failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_merge( messages::DirChunk* msg )
{
    namespace fs = boost::filesystem;
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        Path_t parentPath = msg->path();

        // get the fileId
        int64_t parentId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % msg->path(), soci::into(parentId);

        for(int i=0; i < msg->entries_size(); i++)
        {
            const messages::DirEntry& entry = msg->entries(i);
            sql << boost::format(
                "INSERT OR IGNORE INTO files (parent,node,path,subscribed) "
                "VALUES(%d,'%s','%s',0) " )
                % parentId
                % entry.path()
                % (parentPath / entry.path()).string();
        }
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::merge(...) failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_incrementVersion( const Path_t& path )
{
    namespace fs = boost::filesystem;
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        sql << boost::format(
            "UPDATE version SET version=version+1 "
            "WHERE file_id=%d AND peer=0" ) % fileId;
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::incrementVersion('" << path << "') failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_getVersion( const Path_t& path, VersionVector& v )
{
    namespace fs = boost::filesystem;
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        typedef soci::rowset<soci::row> rowset_t;
        rowset_t rowset = (sql.prepare << boost::format(
                "SELECT peer,version FROM version WHERE file_id=%d" )
                % fileId );

        for( auto& row : rowset )
            v[ row.get<int>(0) ] = row.get<int>(1);
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::getVersion('" << path << "') failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_setVersion( const Path_t& path, const VersionVector& v )
{
    namespace fs = boost::filesystem;
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        for( auto& pair : v )
        {
            sql << boost::format(
                    "INSERT OR REPLACE INTO version (file_id,peer,version) "
                    "VALUES (%d,%d,%d)")
                    % fileId
                    % pair.first
                    % pair.second;
        }
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::setVersion('" << path << "') failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}

void Database::lockless_assimilateKeys( const Path_t& path, const VersionVector& v)
{
    namespace fs = boost::filesystem;
    using namespace soci;

    // create sqlite connection
    session sql(soci::sqlite3, m_dbFile.string() );

    try
    {
        int64_t fileId;
        sql << boost::format("SELECT id FROM files WHERE path='%s'")
                % path.string(), soci::into(fileId);

        for( auto& pair : v )
        {
            sql << boost::format(
                    "INSERT OR IGNORE INTO version (file_id,peer,version) "
                    "VALUES (%d,%d,%d)")
                    % fileId
                    % pair.first
                    % 0;
        }
    }
    catch( const std::exception& ex )
    {
        std::stringstream report;
        report << "Database::assimilateKeys('" << path << "') failed:\n"
               << ex.what() << "\n";
        std::cerr << report.str();
    }
}


void Database::mknod( const Path_t& path )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_mknod(path);
}

void Database::unlink( const Path_t& path )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_unlink(path);
}

void Database::readdir( const Path_t& path,
                void *buf, fuse_fill_dir_t filler, off_t offset )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_readdir(path,buf,filler,offset);
}

void Database::readdir( const Path_t& path,
                messages::DirChunk* msg )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_readdir(path,msg);
}

void Database::readdir( const Path_t& path,
                std::list<std::string>& listing,
                bool subscribed )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_readdir(path,listing,subscribed);
}

void Database::merge( messages::DirChunk* msg )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_merge(msg);
}

void Database::incrementVersion( const Path_t& path )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_incrementVersion(path);
}

void Database::getVersion( const Path_t& path, VersionVector& v )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_getVersion(path,v);
}

void Database::setVersion( const Path_t& path, const VersionVector& v )
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_setVersion(path,v);
}

void Database::assimilateKeys( const Path_t& path, const VersionVector& v)
{
    pthreads::ScopedLock lock(m_mutex);
    lockless_assimilateKeys(path,v);
}


} //< namespace filesystem
} //< namespace openbook




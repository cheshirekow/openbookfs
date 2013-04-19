/**
 *  @file   /openbook_fs/[Source directory]/src/backend/MetaFile.cpp
 *
 *  @date   Apr 19, 2013
 *  @author Josh Bialkowski <jbialk@mit.edu>
 *  @brief  
 */

#include <ctime>
#include <fcntl.h>

#include <soci/sqlite3/soci-sqlite3.h>

#include "MetaFile.h"
#include "TimeSpec.h"



namespace   openbook {
namespace filesystem {

MetaFile::MetaFile( const Path_t& path ):
    m_sql(soci::sqlite3,path.string())
{

}

MetaFile::~MetaFile()
{

}

void MetaFile::init()
{
    m_sql << "CREATE TABLE IF NOT EXISTS meta ("
                "key TEXT UNIQUE NOT NULL, "
                "value)";

    // note: states are
    // 0: synced
    // 1: dirty
    // 2: stale
    // 3: conflict
    m_sql << "INSERT OR IGNORE INTO meta (key,value) values ('state',0)";

    // create
    m_sql << "CREATE TABLE IF NOT EXISTS version ("
            "client TEXT UNIQUE NOT NULL, "
            "version INTEGER NOT NULL) ";

    // insert the version entry for "me"
    m_sql << "INSERT INTO version (client,version) VALUES (0,0)";

    // note: file types are
    // S_IFREG  : regular file
    // S_IFDIR  : subdirectory
    // S_IFIFO  : named pipe
    // S_IFSOCK : unix socket
    // S_IFSYM  : symbolic link
    //          : hard link (not yet supported)
    m_sql << "CREATE TABLE IF NOT EXISTS entries ("
            "path VARCHAR(255) UNIQUE NOT NULL, "
            "type INTEGER NOT NULL DEFAULT(1), "
            "mode INTEGER NOT NULL",
            "subscribed INTEGER NOT NULL, "
            "size INTEGER NOT NULL, "
            "ctime_sec INTEGER NOT NULL, "
            "ctime_nsec INTEGER NOT NULL, "
            "mtime_sec INTEGER NOT NULL, "
            "mtime_nsec INTEGER NOT NULL) ";
}

void MetaFile::mknod( const std::string& path, mode_t type, mode_t mode )
{
    TimeSpec now;
    clock_gettime( CLOCK_REALTIME, now.ptr() );

    // insert the entry if the file is in fact new
    m_sql << "INSERT OR IGNORE INTO entries ("
                "path, "
                "type, "
                "mode, "
                "subscribed, "
                "size, "
                "ctime_sec, "
                "ctime_nsec, "
                "mtime_sec, "
                "mtime_nsec, "
                ") VALUES ("
                << "'" << path << "', "
                << type << ", "
                << mode << ", "
                << 0 << ", "
                << 0 << ", "
                << now.sec()  << ", "
                << now.nsec() << ", "
                << now.sec()  << ", "
                << now.nsec() <<
                ")";

    // update subscription to the file
    m_sql << "UPDATE entries SET subscribed=1 WHERE path='"
          << path << "'";
}


} //< namespace filesystem
} //< namespace openbook

/**
 *  @file   /openbook_fs/[Source directory]/src/backend/MetaFile.cpp
 *
 *  @date   Apr 19, 2013
 *  @author Josh Bialkowski <jbialk@mit.edu>
 *  @brief  
 */

#include <ctime>
#include <fcntl.h>

#include <boost/tuple/tuple.hpp>
#include <soci/boost-tuple.h>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "MetaFile.h"
#include "TimeSpec.h"
#include "ExceptionStream.h"


namespace   openbook {
namespace filesystem {

MetaFile::MetaFile( const Path_t& path ):
    m_sql(soci::sqlite3, (path/"obfs.sqlite").string())
{

}

MetaFile::~MetaFile()
{

}

void MetaFile::init()
{
    // create
    m_sql << "CREATE TABLE IF NOT EXISTS version ("
            "path VARCHAR (255) NOT NULL, "
            "client INTEGER UNIQUE NOT NULL, "
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
//            "type INTEGER NOT NULL DEFAULT(1), "
//            "mode INTEGER NOT NULL, "
            "subscribed INTEGER NOT NULL )";
}

void MetaFile::mknod( const std::string& path ) //, mode_t type, mode_t mode )
{
    // insert the entry if the file is in fact new
    m_sql << "INSERT OR IGNORE INTO entries ("
                "path, "
//                "type, "
//                "mode, "
                "subscribed "
                ") VALUES ("
                << "'" << path << "', "
//                << type << ", "
//                << mode << ", "
                << 0    <<
                ")";

    // update subscription to the file
    m_sql << "UPDATE entries SET subscribed=1 WHERE path='"
          << path << "'";

    // set the initial version of the file
    m_sql << "INSERT OR IGNORE INTO version ("
                "path, "
                "client, "
                "version, "
                ") VALUES ("
                << "'" << path << "', "
                << 0 << ", "
                << 0 <<
                ")";

    incrementVersion();
}

void MetaFile::unlink( const std::string& path )
{
    m_sql << "DELETE FROM entries WHERE path='" << path << "'";
    m_sql << "DELETE FROM version WHERE path='" << path << "'";
    incrementVersion();
}

void MetaFile::readdir( void *buf, fuse_fill_dir_t filler, off_t offset )
{
    typedef boost::tuple<std::string,int> row_t;
    typedef soci::rowset<row_t>           rowset_t;
    rowset_t rs = m_sql.prepare
        << "SELECT path,type FROM entries ORDER BY path LIMIT -1 OFFSET "
        << offset;

    for( auto& row : rs )
    {
        const std::string& path = row.get<0>();
        const int          type = row.get<1>();

        if( filler(buf,path.c_str(),NULL,++offset) )
            return;
    }
}

void MetaFile::incrementVersion()
{
    m_sql << "UPDATE version SET version=version+1 WHERE client=0";
}

void MetaFile::incrementVersion( const std::string& path )
{
    m_sql << "UPDATE version SET version=version+1 WHERE path='"
          << path << "' AND client=0";
}


} //< namespace filesystem
} //< namespace openbook

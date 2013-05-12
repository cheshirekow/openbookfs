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
#include <boost/format.hpp>
#include <soci/boost-tuple.h>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "MetaFile.h"
#include "TimeSpec.h"
#include "ExceptionStream.h"


namespace   openbook {
namespace filesystem {

MetaFile::MetaFile( const Path_t& path )
{
    namespace fs = boost::filesystem;
    Path_t metapath;

    if( fs::is_directory(path) )
    {
        metapath = path / "obfs.sqlite";
        m_subpath = ".";
    }
    else
    {
        metapath = path.parent_path() / "obfs.sqlite";
        m_subpath = path.filename().string();
    }
    m_sql.open( soci::sqlite3, metapath.string() );
}

MetaFile::~MetaFile()
{

}

void MetaFile::init()
{
    // create
    m_sql << "CREATE TABLE IF NOT EXISTS version ("
            "path VARCHAR (255) NOT NULL, "
            "client INTEGER NOT NULL, "
            "version INTEGER NOT NULL, "
            "PRIMARY KEY (path,client) ) ";

    // insert the version entry for "me"
    m_sql << "INSERT OR IGNORE INTO version "
                "(path,client,version) VALUES ('.',0,0)";

    // create the directory entries
    m_sql << "CREATE TABLE IF NOT EXISTS entries ("
            "path VARCHAR(255) UNIQUE NOT NULL, "
            "subscribed INTEGER NOT NULL )";
}

void MetaFile::mknod( const std::string& path ) //, mode_t type, mode_t mode )
{
    // insert the entry if the file is in fact new
    m_sql << "INSERT OR IGNORE INTO entries ("
                "path, "
                "subscribed "
                ") VALUES ("
                << "'" << path << "', "
                << 0    <<
                ")";

    // update subscription to the file
    m_sql << "UPDATE entries SET subscribed=1 WHERE path='"
          << path << "'";

    // set the initial version of the file
    m_sql << "INSERT OR IGNORE INTO version ("
                "path, "
                "client, "
                "version "
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
    typedef soci::rowset<std::string> rowset_t;
    rowset_t rs = m_sql.prepare
        << "SELECT path FROM entries ORDER BY path LIMIT -1 OFFSET "
        << offset;

    for( auto& path : rs )
    {
        if( filler(buf,path.c_str(),NULL,++offset) )
            return;
    }
}

void MetaFile::readdir( messages::DirChunk* msg )
{
    typedef soci::rowset<std::string> rowset_t;
    rowset_t rs = m_sql.prepare
        << "SELECT path FROM entries ORDER BY path";

    for( auto& path : rs )
    {
        messages::DirEntry* entry = msg->add_entries();
        entry->set_path(path);
    }
}

void MetaFile::merge( messages::DirChunk* msg )
{
    for(int i=0; i < msg->entries_size(); i++)
    {
        const messages::DirEntry& entry = msg->entries(i);
        m_sql << "INSERT OR IGNORE INTO entries (path,subscribed) "
                 "VALUES ('" << entry.path() << "', 0)";
    }
}

void MetaFile::incrementVersion()
{
    incrementVersion( m_subpath );
}

void MetaFile::incrementVersion( const std::string& path )
{
    m_sql << "UPDATE version SET version=version+1 WHERE path='"
          << path << "' AND client=0";
}

void MetaFile::getVersion( const std::string& path, VersionVector& v )
{
    typedef soci::rowset<soci::row> rowset_t;
    rowset_t rowset = (m_sql.prepare
            << "SELECT client,version FROM version WHERE path='" << path << "'" );

    for( auto& row : rowset )
        v[ row.get<int>(0) ] = row.get<int>(1);
}

void MetaFile::assimilateKeys( const std::string& path, const VersionVector& v)
{
    for( auto& pair : v )
    {
        m_sql << boost::format(
                "INSERT OR IGNORE INTO version (path,client,version) "
                " VALUES ('%s',%d,%d) ")
                % path
                % pair.first
                % 0;
    }
}


} //< namespace filesystem
} //< namespace openbook

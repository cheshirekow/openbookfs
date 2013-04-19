/**
 *  @file   /openbook_fs/[Source directory]/src/backend/MetaFile.h
 *
 *  @date   Apr 19, 2013
 *  @author Josh Bialkowski <jbialk@mit.edu>
 *  @brief  
 */

#ifndef METAFILE_H_
#define METAFILE_H_

#include <boost/filesystem.hpp>
#include <soci/soci.h>

namespace   openbook {
namespace filesystem {

/// wraps access to the sqlite database for a file and provides methods
/// which implement SQL queries
class MetaFile
{
    public:
        typedef boost::filesystem::path Path_t;
        typedef soci::session           Session_t;

    private:
        Session_t m_sql;

    public:
        /// open a soci session to the sqlite meta file for the specified
        /// path
        MetaFile( const Path_t& path );

        /// closes the sqlite metadata file
        ~MetaFile();

        /// initialize the metadata by creating appropriate tables
        void init();

        /// add an entry to the file list for
        void mknod( const std::string& path, mode_t type, mode_t mode );

        /// remove an entry from the file list
        void unlink( const std::string& path );

        /// increase the version vector for entry 0 (this)
        void incrementVersion();


};


} //< namespace filesystem
} //< namespace openbook




#endif /* METAFILE_H_ */

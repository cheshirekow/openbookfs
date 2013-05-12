/**
 *  @file   /openbook_fs/[Source directory]/src/backend/Database.h
 *
 *  @date   Apr 19, 2013
 *  @author Josh Bialkowski <jbialk@mit.edu>
 *  @brief  
 */

#ifndef OPENBOOK_FS_DATABASE_H_
#define OPENBOOK_FS_DATABASE_H_

#include <map>
#include <string>

#include <boost/filesystem.hpp>
#include <cpp-pthreads.h>

#include "fuse_include.h"
#include "messages.pb.h"
#include "Synchronized.h"
#include "VersionVector.h"



namespace   openbook {
namespace filesystem {

/// wraps access to the main sqlite database
class Database
{
    public:
        typedef boost::filesystem::path Path_t;

        typedef std::map<std::string,int>   USIdMap_t;
        typedef Synchronized<USIdMap_t>     IdMap_t;

    private:
        Path_t          m_dbFile;
        pthreads::Mutex m_mutex;

    public:
        Database( );

        ~Database();

        /// set the location of the databse file
        void setPath( const Path_t& path );

        /// initialize the database by creating appropriate tables if they
        /// dont already exists
        void init();

        /// retrieve the client map from the database
        void getClientMap( LockedPtr<USIdMap_t>& map );

        /// register and retrieve the peerId given the peers public key
        int registerPeer( const std::string& publicKey,
                          const std::string& displayName );

        /// fill a peer map message
        void buildPeerMap( messages::IdMap* map );

        /// adds the requested path for download or pre-empts a current
        /// download if a newer version is to be retrieved
        void addDownload( int64_t peer,
                            const Path_t& path,
                            int64_t size,
                            const VersionVector& version,
                            const Path_t& stageDir );

        /// merge a file chunk into a staging file
        void mergeData( int64_t peer,
                        const Path_t& stageDir,
                        const Path_t& rootDir,
                        messages::FileChunk* chunk );

        //-----------------MetaFile Replacement API-------------------------
        /// add an entry to the file list for
        void mknod( const Path_t& path );

        /// remove an entry from the file list
        void unlink( const Path_t& path );

        /// read directory entries into a fuse buffer
        void readdir( const Path_t& path,
                        void *buf, fuse_fill_dir_t filler, off_t offset );

        /// read directory entries into a message
        void readdir( const Path_t& path,
                        messages::DirChunk* msg );

        /// merge entries from another peer
        void merge( messages::DirChunk* msg );

        /// increase the version vector for entry 0 (this peer)
        void incrementVersion( const Path_t& path );

        /// get the version for a path
        void getVersion( const Path_t& path, VersionVector& v );

        /// set the version for a path
        void setVersion( const Path_t& path, const VersionVector& v );

        /// assimilate keys for the specified path, any keys which we dont
        /// already have are set to version value of zero
        void assimilateKeys( const Path_t& path, const VersionVector& v);




};


} //< namespace filesystem
} //< namespace openbook




#endif /* DATABASE_H_ */

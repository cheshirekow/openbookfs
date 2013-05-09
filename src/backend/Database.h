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

#include "messages.pb.h"
#include "Synchronized.h"



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




};


} //< namespace filesystem
} //< namespace openbook




#endif /* DATABASE_H_ */

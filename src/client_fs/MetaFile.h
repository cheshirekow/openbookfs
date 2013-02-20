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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/MetaFile.h
 *
 *  @date   Feb 18, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_METAFILE_H_
#define OPENBOOK_METAFILE_H_

#include <stdint.h>
#include "ExtendedAttributes.h"

namespace   openbook {
namespace filesystem {
namespace     client {


/// private stuff for meta data file
namespace       meta {


/// numeric constant which maps to a std::string used for the STATE xattr
enum State
{
    SYNCED,     ///< the file is synchronized
    DIRTY,      ///< the file has been changed by the user
    STALE,      ///< the file has been changed on the server
    CONFLICT,   ///< the file has irreconcilable changes
    DELETED,    ///< the file has been deleted already, abort! abort!
    NUM_STATES
};

/// meta data file structure
struct File
{
    uint64_t    baseVersion;      ///< last synchronized version
    uint64_t    clientVersion;    ///< number of changes on client
    State       state;            ///< state of the file
};

/// meta data file wrapper
class Data
{
    int     m_fd;       ///< file descriptor of the meta data file
    File*   m_meta;     ///< casted pointer to the mapped meta data file

    // the path to the meta file, mostly for error checking
    boost::filesystem::path m_path;

    public:
        Data(const boost::filesystem::path& path):
            m_fd(0),
            m_meta(0),
            m_path(path)
        {}

        void create( )
        {
            // open the file
            m_fd = ::open( m_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
            if( m_fd < 0 )
                ex()() << "Failed to open meta file " << m_path.string()
                       << " (" << errno << "): " << strerror(errno);

            // truncate the file
            if( ftruncate( m_fd, sizeof(File) ) )
                ex()() << "Failed to truncate meta file " << m_path.string()
                       << " (" << errno << "): " << strerror(errno);

            // optain an exclusive lock
            if( flock(m_fd, LOCK_EX ) )
                ex()() << "Failed to lock meta file " << m_path.string()
                       << " (" << errno << "): " << strerror(errno);


            void* ptr = mmap( 0, sizeof(File), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0 );
            if( ptr == MAP_FAILED )
                ex()() << "Failed to map meta file " << m_path.string();

            m_meta = static_cast<File*>(ptr);

            m_meta->baseVersion   = 0;
            m_meta->clientVersion = 0;
            m_meta->state         = SYNCED;
        }


        void open(  )
        {
            // open the file
            m_fd = ::open( m_path.c_str(), O_RDWR, S_IRUSR | S_IWUSR );
            if( m_fd < 0 )
                ex()() << "Failed to open meta file " << m_path.string()
                       << " (" << errno << "): " << strerror(errno);

            // optain an exclusive lock
            int result = flock(m_fd, LOCK_EX );
            if( result )
                ex()() << "Failed to lock meta file " << m_path.string()
                       << " ("<< errno << "): " << strerror(errno);


            void* ptr = mmap( 0, sizeof(File), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0 );
            if( ptr == MAP_FAILED )
                ex()() << "Failed to map meta file " << m_path.string();

            m_meta = static_cast<File*>(ptr);
        }

        void close()
        {
            if(m_meta)
                munmap(m_meta,sizeof(File));

            // note: will also unlock
            if(m_fd)
                ::close(m_fd);

            m_meta = 0;
            m_fd   = 0;
        }

        uint64_t baseVersion(){ return m_meta->baseVersion; }
        void set_baseVersion( uint64_t version )
            { m_meta->baseVersion = version; }

        uint64_t clientVersion(){ return m_meta->clientVersion; }
        void set_clientVersion( uint64_t version )
            { m_meta->clientVersion = version; }

        State state(){ return m_meta->state; }
        void set_state( State state )
            { m_meta->state = state; }

};




} // namespace meta
} // namespace client
} // namespace filesystem
} // namespace openbook



namespace   openbook {
namespace filesystem {
namespace     client {


typedef meta::Data MetaData;

} // namespace client
} // namespace filesystem
} // namespace openbook











#endif // METAFILE_H_

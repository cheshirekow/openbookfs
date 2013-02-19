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

#include <cerrno>

#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "ExceptionStream.h"

namespace   openbook {
namespace filesystem {

/// private stuff for meta data file
namespace       meta {


/// numeric constant which maps to a std::string used for the STATE xattr
enum State
{
    SYNCED,         ///< the file is synchronized
    INCOMPLETE,     ///< the file has not completely downloaded yet from some
                    ///  client
};

struct Subscription
{
    static const unsigned int KEY_TRUNC;
    const char  pubkey[KEY_TRUNC]; ///< base64 encoded public key (truncated)
};

/// mmapp-able structure of a meta file
struct File
{
    uint64_t        version;          ///< official version number
    State           state;            ///< state of the file
    unsigned int    nSubscribed;      ///< number of subscriptions
    Subscription    subs;             ///< subscription array
};

/// meta data file structure, note: locks the file until destroyed
struct Data
{
    uint64_t        version;          ///< official version number
    State           state;            ///< state of the file
    int             fd;               ///< file descriptor
    std::vector<Subscription> subs;   ///< subscription array

    /// on construction, locks and loads the file
    Data(const std::string& path)
    {
        fd = open(path.c_str(), O_RDWR );
        if( fd < 0 )
            ex()() << "Failed to open meta file " << path << ", (" << errno
                   << ") : " << strerror(errno);

        // aquire lock on the meta data file, note locks are process + fd
        // specific. Since this FD is currently unique to this object, it will
        // block if someone else has a lock on the meta data object
        int result = flock(fd,LOCK_EX);

        // map the file
        struct stat file_stat;
        int fstat( fd, &file_stat );

        void* ptr = mmap( 0, file_stat.st_size, PROT_WRITE, MAP_SHARED, fd, 0 );
        if( ptr == MAP_FAILED )
            ex()() << "Failed to map meta file " << path << ", (" << errno
                   << ") : " << strerror(errno);

        File* metaFile = static_cast<File*>(ptr);

        // verify the size
        if( file_stat.st_size != sizeof(File) + (metaFile->nSubscribed-1)*sizeof(Subscription))
        {
            munmap(ptr,file_stat.st_size);
            ex()() << "Corrupt meta file " << path << " has size "
                   << file_stat.st_size << " but it has "
                   << metaFile->nSubscribed << " subscriptions so the size "
                   << " should be "
                   << sizeof(File) +
                       (metaFile->nSubscribed-1)*sizeof(Subscription);
        }

        // copy data into the structure
        version = metaFile->version;
        state   = metaFile->state;
        subs.reserve( metaFile->nSubscribed );
        for(int i=0; i < metaFile->nSubscribed; i++)
            subs.push_back( (&(metaFile->subs))[i] );

        // unmap the file
        munmap(ptr,file_stat.st_size);
    }

    /// on destruction, releases and flushes the meta data
    ~Data()
    {
        // truncate the file to the new size
        size_t size = (subs.size()-1)*sizeof(Subscription);
        int result =
            ftruncate(fd,sizeof(File) + size);
        if( result < 0 )
        {
            close(fd);
            ex()() << "Failed to truncate the metafile to size " << size;
        }

        // remap the file
        void* ptr = mmap( 0, size, PROT_WRITE, MAP_SHARED, fd, 0 );
        if( ptr == MAP_FAILED )
        {
            close(fd);
            ex()() << "Failed to map meta file for flush, (" << errno
                   << ") : " << strerror(errno);
        }

        File* metaFile = static_cast<File*>(ptr);

        // copy data into the structure
        metaFile->version       = version;
        metaFile->state         = state;
        metaFile->nSubscribed   = subs.size();
        subs.reserve( metaFile->nSubscribed );
        for(int i=0; i < subs.size(); i++)
            (&(metaFile->subs))[i] = subs[i];

        close(fd);
    }



};




} // namespace meta
} // namespace filesystem
} // namespace openbook



namespace   openbook {
namespace filesystem {

typedef meta::Data MetaData;

} // namespace filesystem
} // namespace openbook











#endif // METAFILE_H_

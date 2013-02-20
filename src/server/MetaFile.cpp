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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/MetaFile.cpp
 *
 *  @date   Feb 20, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include <cstring>
#include "MetaFile.h"

namespace   openbook {
namespace filesystem {
 namespace    server {
namespace       meta {

Data::Data(const std::string& path ):
    m_path(path)
{
    m_baseVersion   = 0;
    m_clientVersion = 1;
    m_owner         = 0;
    m_state         = SYNCED;
    m_fd            = 0;
}

/// on destruction, releases and flushes the meta data
void Data::create()
{
    // open the file
    m_fd = ::open(m_path.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR );
    if( m_fd < 0 )
        ex()() << "Failed to open meta file " << m_path << ", (" << errno
               << ") : " << strerror(errno);

    // aquire lock on the meta data file, note locks are process + fd
    // specific. Since this FD is currently unique to this object, it will
    // block if someone else has a lock on the meta data object
    int result = flock(m_fd,LOCK_EX);

    // truncate the file to the new size
    size_t size = sizeof(File) + (subs.size()-1)*sizeof(uint64_t);
    result = ftruncate(m_fd,size);
    if( result < 0 )
    {
        ::close(m_fd);
        ex()() << "Failed to truncate the metafile to size " << size;
    }

    // remap the file
    void* ptr = mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0 );
    if( ptr == MAP_FAILED )
    {
        ::close(m_fd);
        ex()() << "Failed to map meta file for create, (" << errno
               << ") : " << strerror(errno);
    }

    File* metaFile = static_cast<File*>(ptr);

    // copy data into the structure
    metaFile->baseVersion       = m_baseVersion;
    metaFile->clientVersion     = m_clientVersion;
    metaFile->state         = m_state;
    metaFile->nSubscribed   = subs.size();

    std::set<uint64_t>::iterator iID;
    int i=0;
    for(iID = subs.begin(); iID != subs.end(); ++iID)
        (&(metaFile->subs))[i++] = *iID;

    // unmap the file
    munmap( ptr, size );

    ::close(m_fd);
}

void Data::load()
{
    m_fd = ::open(m_path.c_str(), O_RDWR );
    if( m_fd < 0 )
        ex()() << "Failed to open meta file " << m_path << ", (" << errno
               << ") : " << strerror(errno);

    // aquire lock on the meta data file, note locks are process + fd
    // specific. Since this FD is currently unique to this object, it will
    // block if someone else has a lock on the meta data object
    int result = flock(m_fd,LOCK_EX);

    // map the file
    struct stat file_stat;
    result = fstat( m_fd, &file_stat );
    if( result )
        ex()() << "Failed to stat meta file " << m_path;

    void* ptr = mmap( 0, file_stat.st_size,
                        PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0 );

    if( ptr == MAP_FAILED )
        ex()() << "Failed to map meta file " << m_path << ", (" << errno
               << ") : " << strerror(errno);

    File* metaFile = static_cast<File*>(ptr);

    // verify the size
    if( file_stat.st_size != sizeof(File)
            + (metaFile->nSubscribed-1)*sizeof(uint64_t))
    {
        munmap(ptr,file_stat.st_size);
        ex()() << "Corrupt meta file " << m_path << " has size "
               << file_stat.st_size << " but it has "
               << metaFile->nSubscribed << " subscriptions so the size "
               << " should be "
               << sizeof(File) +
                   (metaFile->nSubscribed-1)*sizeof(uint64_t);
    }

    // copy data into the structure
    m_baseVersion    = metaFile->baseVersion;
    m_clientVersion  = metaFile->clientVersion;
    m_owner   = metaFile->owner;
    m_state   = metaFile->state;
    for(int i=0; i < metaFile->nSubscribed; i++)
        subs.insert( (&(metaFile->subs))[i] );

    // unmap the file
    munmap(ptr,file_stat.st_size);
}

void Data::flush()
{
    // truncate the file to the new size
    size_t size = (subs.size()-1)*sizeof(uint64_t);
    int result =
        ftruncate(m_fd,sizeof(File) + size);
    if( result < 0 )
    {
        ::close(m_fd);
        ex()() << "Failed to truncate the metafile to size " << size;
    }

    // remap the file
    void* ptr = mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0 );
    if( ptr == MAP_FAILED )
    {
        ::close(m_fd);
        ex()() << "Failed to map meta file for flush, (" << errno
               << ") : " << strerror(errno);
        EAGAIN;
    }

    File* metaFile = static_cast<File*>(ptr);

    // copy data into the structure
    metaFile->baseVersion       = m_baseVersion;
    metaFile->clientVersion     = m_clientVersion;
    metaFile->state         = m_state;
    metaFile->nSubscribed   = subs.size();
    std::set<uint64_t>::iterator iID;
    int i=0;
    for(iID = subs.begin(); iID != subs.end(); ++iID)
        (&(metaFile->subs))[i++] = *iID;

    // unmap the file
    munmap( ptr, size );

    ::close(m_fd);
}

void Data::subscribe( uint64_t id )
{
    subs.insert(id);
}

void Data::unsubscribe( uint64_t id )
{
    subs.erase(id);
}


} // namespace meta
} // namespace server 
} // namespace filesystem
} // namespace openbook









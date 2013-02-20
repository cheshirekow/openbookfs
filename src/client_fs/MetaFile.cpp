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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/MetaFile.cpp
 *
 *  @date   Feb 20, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "MetaFile.h"
#include "ExceptionStream.h"


namespace   openbook {
namespace filesystem {
namespace     client {
namespace       meta {

Data::Data( const boost::filesystem::path& path ):
    m_fd(0),
    m_meta(0),
    m_path(path)
{}


void Data::load(  )
{
    // open the file
    m_fd = ::open( m_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if( m_fd < 0 )
        ex()() << "Failed to open meta file " << m_path.string()
               << " (" << errno << "): " << strerror(errno);

    // optain an exclusive lock
    int result = flock(m_fd, LOCK_EX );
    if( result )
    {
        ::close(m_fd);
        ex()() << "Failed to lock meta file " << m_path.string()
               << " ("<< errno << "): " << strerror(errno);
    }

    // if the file size is too small then we should resize it
    struct stat fs;
    result = fstat(m_fd,&fs);
    if( result )
    {
        ::close(m_fd);
        ex()() << "Failed to stat meta file "
               << m_path.string()
               << " ("<< errno << "): " << strerror(errno);
    }

    bool isNew = fs.st_size < sizeof(File);
    if( isNew )
    {
        result = ftruncate(m_fd,sizeof(File));
        if( result )
        {
            ::close(m_fd);
            ex()() << "Failed to truncate meta file "
                   << m_path.string()
                   << " ("<< errno << "): " << strerror(errno);
        }
    }

    void* ptr = mmap( 0, sizeof(File), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0 );
    if( ptr == MAP_FAILED )
        ex()() << "Failed to map meta file " << m_path.string();

    m_meta = static_cast<File*>(ptr);

    if(isNew)
    {
        m_meta->baseVersion   = 0;
        m_meta->clientVersion = 0;
        m_meta->state         = SYNCED;
    }
}

void Data::flush()
{
    if(m_meta)
        munmap(m_meta,sizeof(File));

    // note: will also unlock
    if(m_fd)
        ::close(m_fd);

    m_meta = 0;
    m_fd   = 0;
}



} // namespace meta
} // namespace client
} // namespace filesystem
} // namespace openbook



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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/FileContext.cpp
 *
 *  @date   Apr 21, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "FileContext.h"
#include "ExceptionStream.h"

namespace   openbook {
namespace filesystem {

FileContext::FileContext( const Path_t& path, int fd ):
    m_fd(fd),
    m_meta(path),
    m_changed(false)
{

}

FileContext::~FileContext()
{
    ::close(m_fd);
    if(m_changed)
        m_meta.incrementVersion();
}

RefPtr<FileContext> FileContext::create( const Path_t& path, int fd )
{
    return new FileContext(path,fd);
}



FileMap::FileMap( int size )
{
    m_mutex.init();
    m_fileVec.resize(size,0);
    m_freeStore.reserve(size);

    for(int i=0; i < size; i++)
        m_freeStore.push_back(i);
}

FileMap::~FileMap()
{

}

RefPtr<FileContext> FileMap::operator[](int i)
{
    pthreads::ScopedLock lock( m_mutex );

    if( i < 0 || i >= m_fileVec.size() )
        return (FileContext*)0;
    return m_fileVec[i];
}

int FileMap::registerFile( const Path_t& path, int os_fd )
{
    pthreads::ScopedLock lock( m_mutex );

    if( m_freeStore.size() < 1 )
        ex()() << "No available file descriptors";

    int fd = m_freeStore.back();
    m_freeStore.pop_back();

    try
    {
        m_fileVec[fd] = FileContext::create(path,os_fd);
    }
    catch( const std::exception& ex )
    {
        m_freeStore.push_back(fd);
        throw;
    }

    return fd;
}

void FileMap::unregisterFile( int fd )
{
    pthreads::ScopedLock lock( m_mutex );

    m_fileVec[fd].clear();
    m_freeStore.push_back(fd);
}





} //< filesystem
} //< openbook









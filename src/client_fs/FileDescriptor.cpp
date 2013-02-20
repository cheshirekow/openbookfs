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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/FileDescriptor.cpp
 *
 *  @date   Feb 20, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "FileDescriptor.h"


namespace   openbook {
namespace filesystem {

FileDescriptor::FileDescriptor()
{
    m_mutex.init();
    m_flags[fd::FLAG_OPENED]  = false;
    m_flags[fd::FLAG_CHANGED] = false;
}

FileDescriptor::~FileDescriptor()
{
    m_mutex.destroy();
}

pthreads::Mutex& FileDescriptor::mutex()
{
    return m_mutex;
}

void FileDescriptor::open()
{
    m_flags[fd::FLAG_OPENED]  = true;
    m_flags[fd::FLAG_CHANGED] = false;
}

bool FileDescriptor::flag( fd::Flags flag )
{
    return m_flags[flag];
}

void FileDescriptor::flag( fd::Flags flag, bool state)
{
    m_flags[flag] = state;
}

} // namespace filesystem
} // namespace openbook




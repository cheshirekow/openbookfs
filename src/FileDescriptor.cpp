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
 *  @file   src/backend/FileDescriptor.cpp
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "FileDescriptor.h"
#include "iostream"

namespace   openbook {
namespace filesystem {


FileDescriptor::FileDescriptor(int fd):
    fd(fd)
{
    std::cout << "FileDescriptor: initialized refcount for fd: " << fd << "\n";
}

FileDescriptor::~FileDescriptor()
{
    std::cout << "FileDescriptor: closing fd: " << fd << "\n";
    close(fd);
}

FileDescriptor::operator int() const
{
    return fd;
}

RefPtr<FileDescriptor> FileDescriptor::create( int fd )
{
    return RefPtr<FileDescriptor>( new FileDescriptor(fd) );
}


} // namespace filesystem
} // namespace openbook







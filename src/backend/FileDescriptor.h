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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/FileDescriptor.h
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_FILEDESCRIPTOR_H_
#define OPENBOOK_FS_FILEDESCRIPTOR_H_

#include <unistd.h>
#include "ReferenceCounted.h"

namespace   openbook {
namespace filesystem {

/// adds a reference count to a file descriptor and closes the fd when
/// destroyed
class FileDescriptor:
    public ReferenceCounted
{
    private:
        int fd;

        FileDescriptor(int fd):
            fd(fd)
        {}

    public:
        ~FileDescriptor()
        {
            close(fd);
        }

        operator int() const
        {
            return fd;
        }

        static RefPtr<FileDescriptor> create( int fd )
        {
            return RefPtr<FileDescriptor>( new FileDescriptor(fd) );
        }
};


}
}















#endif // FILEDESCRIPTOR_H_

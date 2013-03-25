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
 *  @file   src/NotifyPipe.cpp
 *
 *  @date   Feb 13, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include "NotifyPipe.h"
#include "ExceptionStream.h"


namespace   openbook {
namespace filesystem {

NotifyPipe::NotifyPipe()
{
    if( pipe(m_fd) )
        ex()() << "Failed to open a pipe";

    // make the read end nonblocking
    if( fcntl(readFd(), F_SETFL, O_NONBLOCK) )
        ex()() << "Failed to make pipe nonblocking";
}

NotifyPipe::~NotifyPipe()
{
    for(int i=0; i < 2; i++)
        close(m_fd[i]);
}

int NotifyPipe::writeFd()
{
    return m_fd[1];
}

int NotifyPipe::readFd()
{
    return m_fd[0];
}

void NotifyPipe::notify()
{
    write( writeFd(), "x", 1 );
}

void NotifyPipe::clear()
{
    char c;
    int  result = 0;

    // read until there's nothing left
    while( result >= 0 )
        result = read( readFd(), &c, 1 );

    if( errno != EWOULDBLOCK )
        ex()() << "problem clearing notify pipe: " << errno
               << " : " << strerror(errno) << std::endl;
}


} // namespace filesystem
} // namespace openbook

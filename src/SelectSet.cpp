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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/SelectSet.cpp
 *
 *  @date   Feb 13, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "SelectSet.h"
#include "ExceptionStream.h"

namespace   openbook {
namespace filesystem {

SelectSet::SelectSet(int numfd)
{
    m_fd.resize(numfd,0);
    setTimeout(1,0);
    init();
}

int& SelectSet::operator[]( unsigned int i_fd )
{
    if( i_fd >= m_fd.size() )
        ex()() << "Attempt to access file descriptor " << i_fd
               << " in a set of size " << m_fd.size() ;
    return m_fd[i_fd];
}

void SelectSet::setTimeout( unsigned int sec, unsigned long int usec )
{
    m_timeout.tv_sec  = sec;
    m_timeout.tv_usec = usec;
}

void SelectSet::init()
{
    m_maxfd = 0;
    for(unsigned int i=0; i < m_fd.size(); i++)
        m_maxfd = std::max(m_maxfd,m_fd[i]);
}

int SelectSet::wait()
{
    FD_ZERO( &m_set );
    timeval timeout = m_timeout;
    for(unsigned int i=0; i < m_fd.size(); i++)
        FD_SET( m_fd[i], &m_set );

    return select( m_maxfd+1, &m_set, 0, 0, &timeout );
}

bool SelectSet::operator()( unsigned int i_fd )
{
    if( i_fd >= m_fd.size() )
        ex()() << "Attempt to access file descriptor " << i_fd
               << " in a set of size " << m_fd.size() ;
    return FD_ISSET( m_fd[i_fd], &m_set );
}


} // namespace filesystem
} // namespace openbook










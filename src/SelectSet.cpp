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


FdSet::Ref::Ref( fd_set* set, int fd ):
    m_set(set),
    m_fd(fd)
{}

FdSet::Ref::operator bool() const
{
    return FD_ISSET(m_fd, m_set);
}

FdSet::Ref& FdSet::Ref::operator=( bool x )
{
    if( x )
        FD_SET( m_fd,m_set );
    else
        FD_CLR( m_fd, m_set );
    return *this;
}

FdSet::Ref& FdSet::Ref::flip()
{
    if( *this )
        *this = false;
    else
        *this = true;

    return *this;
}

bool FdSet::Ref::operator~() const
{
    return !(*this);
}




FdSet::ConstRef::ConstRef( const fd_set* set, int fd ):
    m_set(set),
    m_fd(fd)
{}

FdSet::ConstRef::operator bool() const
{
    return FD_ISSET(m_fd, m_set);
}

bool FdSet::ConstRef::operator~() const
{
    return !(*this);
}







FdSet::operator fd_set*()
{
    return &m_fdset;
}

FdSet::operator const fd_set*() const
{
    return &m_fdset;
}

void FdSet::clear()
{
    FD_ZERO(&m_fdset);
}

FdSet::Ref FdSet::operator[]( unsigned int fd )
{
    return Ref(&m_fdset,fd);
}

const FdSet::ConstRef FdSet::operator[]( unsigned int fd ) const
{
    return ConstRef(&m_fdset,fd);
}







SelectSet::SelectSet()
{
    setTimeout(0,0);
    init();


}

int& SelectSet::operator[]( unsigned int i_fd )
{
    if( i_fd >= FD_SETSIZE )
        ex()() << "FD_SETSIZE is " << FD_SETSIZE
                << " and there was an attempt to access element " << i_fd;
    if( i_fd >= m_fd.size() )
        m_fd.resize(i_fd+1,0);
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

int SelectSet::wait(Which which)
{
    for(unsigned int i=0; i < NUM_WHICH; i++)
        m_set[i].clear();

    timeval timeout = m_timeout;
    for(unsigned int i=0; i < m_fd.size(); i++)
        m_set[i][m_fd[i]] = true;

    return select( m_maxfd+1, m_set[READ], m_set[WRITE], m_set[EXCEPT], &timeout );
}

bool SelectSet::operator()( unsigned int i_fd, Which which )
{
    if( i_fd >= FD_SETSIZE )
        ex()() << "FD_SETSIZE is " << FD_SETSIZE
                << " and there was an attempt to access element " << i_fd;
    if( i_fd >= m_fd.size() )
        m_fd.resize(i_fd+1,0);

    return m_set[which][m_fd[i_fd]];
}


} // namespace filesystem
} // namespace openbook










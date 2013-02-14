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





SelectSet::Ref::Ref( SelectSet* set, int ifd ):
    m_set(set),
    m_ifd(ifd)
{}

SelectSet::Ref& SelectSet::Ref::clear()
{
    for(int i = READ; i < NUM_WHICH; i++)
        m_set->m_listen[NUM_WHICH*m_ifd + i] = false;
    return *this;
}

SelectSet::Ref& SelectSet::Ref::operator<< (Which which)
{
    m_set->m_listen[NUM_WHICH*m_ifd + which] = true;
    return *this;
}

SelectSet::Ref& SelectSet::Ref::operator, (Which which)
{
    m_set->m_listen[NUM_WHICH*m_ifd + which] = true;
    return *this;
}

SelectSet::Ref& SelectSet::Ref::operator= (int fd)
{
    m_set->m_fd[m_ifd] = fd;
    return *this;
}

bool SelectSet::Ref::operator[](Which which)
{
   return m_set->m_listen[NUM_WHICH*m_ifd + which];
}

SelectSet::Ref::operator int()
{
    return m_set->m_fd[m_ifd] ;
}




SelectSet::SelectSet()
{
    setTimeout(0,0);
    init();


}

SelectSet::Ref SelectSet::operator[]( unsigned int ifd )
{
    if( ifd >= FD_SETSIZE )
        ex()() << "FD_SETSIZE is " << FD_SETSIZE
                << " and there was an attempt to access element " << ifd;
    if( ifd >= m_fd.size() )
    {
        m_fd.resize(ifd+1,0);
        m_listen.resize(NUM_WHICH*(ifd+1),0);
    }

    return Ref(this,ifd);
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
    for(unsigned int i=0; i < NUM_WHICH; i++)
        m_set[i].clear();

    timeval timeout = m_timeout;
    for(unsigned int i=0; i < m_fd.size(); i++)
    {
        int fd = m_fd[i];

        for(unsigned int j=0; j < NUM_WHICH; j++)
            m_set[j][fd] = m_listen[ NUM_WHICH*i + j];
    }

    return select( m_maxfd+1, m_set[READ], m_set[WRITE], m_set[EXCEPT], &timeout );
}

bool SelectSet::operator()( unsigned int ifd, Which which )
{
    if( ifd >= FD_SETSIZE )
        ex()() << "FD_SETSIZE is " << FD_SETSIZE
                << " and there was an attempt to access element " << ifd;
    if( ifd >= m_fd.size() )
    {
        m_fd.resize(ifd+1,0);
        m_listen.resize(NUM_WHICH*(ifd+1),0);
    }

    return m_set[which][m_fd[ifd]];
}


} // namespace filesystem
} // namespace openbook










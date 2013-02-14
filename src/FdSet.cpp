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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/FdSet.cpp
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "FdSet.h"
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




} // namespace filesystem
} // namespace openbook











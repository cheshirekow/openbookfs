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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/SelectSpec.cpp
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include "SelectSpec.h"
#include "ExceptionStream.h"



namespace   openbook {
namespace filesystem {

SelectSpec::Generator::Generator( SelectSpec* spec ):
    m_spec(spec)
{

}


SelectSpec::Generator& SelectSpec::Generator::operator()( int fd, Which which )
{
    m_spec->add(fd, which);
    return *this;
}

SelectSpec::Generator& SelectSpec::Generator::operator()( const TimeVal& to )
{
    m_spec->setTimeout(to);
    return *this;
}

SelectSpec::SelectSpec():
    m_maxfd(0)
{}


void SelectSpec::reset()
{
    m_maxfd = 0;
    m_spec.clear();
}

void SelectSpec::add( int fd, Which which )
{
    m_maxfd = std::max(m_maxfd,fd);
    m_spec.push_back( Elmnt(fd,which) );
}

void SelectSpec::setTimeout( const TimeVal& to )
{
    m_timeout = to;
}

int SelectSpec::wait()
{
    using namespace select_spec;
    for(int i=0; i < NUM_WHICH; i++)
        m_fdset[i].clear();

    for(int i=0; i < m_spec.size(); i++)
        m_fdset[m_spec[i].which][m_spec[i].fd] = true;

    m_remainder = m_timeout;
    int result = select( m_maxfd + 1,
                    m_fdset[READ], m_fdset[WRITE], m_fdset[EXCEPT],
                    m_remainder.ptr() );

    if( result < 0 )
        ex()() << "Problem with select, errno " << errno << " : "
               << strerror( errno );

    return result;
}

bool SelectSpec::ready( int fd, Which which )
{
    return m_fdset[which][fd];
}

SelectSpec::Generator SelectSpec::gen()
{
    return Generator(this);
}





} // namespace filesystem
} // namespace openbook





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
 *  @file   src/TimeSpec.cpp
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "TimeSpec.h"

namespace   openbook {
namespace filesystem {


TimeSpec::TimeSpec( int sec, long int nsec )
{
    m_tv.tv_sec  = sec;
    m_tv.tv_nsec = nsec;
}

TimeSpec::operator timespec&()
{
    return m_tv;
}

TimeSpec::operator const timespec&() const
{
    return m_tv;
}

timespec* TimeSpec::ptr()
{
    return &m_tv;
}

long int& TimeSpec::sec()
{
    return m_tv.tv_sec;
}

const long int& TimeSpec::sec() const
{
    return m_tv.tv_sec;
}

long int& TimeSpec::nsec()
{
    return m_tv.tv_nsec;
}

const long int& TimeSpec::nsec() const
{
    return m_tv.tv_nsec;
}


TimeSpec operator+( const TimeSpec& a, const TimeSpec& b )
{
    TimeSpec result( a.sec() + b.sec(), a.nsec() + b.nsec() );
    if( result.nsec() > 1000000000 )
    {
        result.sec()++;
        result.nsec() -= 1000000000;
    }

    return result;
}

TimeSpec operator-( const TimeSpec& a, const TimeSpec& b )
{
    TimeSpec result(a.sec() - b.sec(),a .nsec() - b.nsec());
    if( result.nsec() < 0 )
    {
        result.sec()--;
        result.nsec() += 1000000000;
    }
    return result;
}



} // namespace filesystem
} // namespace openbook




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
 *  @file   src/TimeVal.cpp
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "TimeVal.h"

namespace   openbook {
namespace filesystem {


TimeVal::TimeVal( int sec, int usec )
{
    m_tv.tv_sec = sec;
    m_tv.tv_usec = usec;
}

TimeVal::operator timeval&()
{
    return m_tv;
}

TimeVal::operator const timeval&() const
{
    return m_tv;
}

timeval* TimeVal::ptr()
{
    return &m_tv;
}

long int& TimeVal::sec()
{
    return m_tv.tv_sec;
}

const long int& TimeVal::sec() const
{
    return m_tv.tv_sec;
}

long int& TimeVal::usec()
{
    return m_tv.tv_usec;
}

const long int& TimeVal::usec() const
{
    return m_tv.tv_usec;
}


TimeVal operator+( const TimeVal& a, const TimeVal& b )
{
    TimeVal result( a.sec() + b.sec(), a.usec() + b.usec() );
    if( result.usec() > 1000000 )
    {
        result.sec()++;
        result.usec() -= 1000000;
    }

    return result;
}

TimeVal operator-( const TimeVal& a, const TimeVal& b )
{
    TimeVal result(a.sec() - b.sec(),a .usec() - b.usec());
    if( result.usec() < 0 )
    {
        result.sec()--;
        result.usec() += 1000000;
    }
    return result;
}




} // namespace filesystem
} // namespace openbook




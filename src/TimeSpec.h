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
 *  @file   src/TimeSpec.h
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_TIMESPEC_H_
#define OPENBOOK_TIMESPEC_H_

#include <sys/time.h>

namespace   openbook {
namespace filesystem {

/// just a c timeval
class TimeSpec
{
    private:
        timespec m_tv;

    public:
        TimeSpec( int sec=0, long int nsec=0 );
        operator timespec&();
        operator const timespec&() const;
        timespec* ptr();

        long int& sec();
        const long int& sec() const;
        long int& nsec();
        const long int& nsec() const;

        TimeSpec& operator+=( const TimeSpec& other );
        TimeSpec& operator-=( const TimeSpec& other );
};


TimeSpec operator+( const TimeSpec& a, const TimeSpec& b );
TimeSpec operator-( const TimeSpec& a, const TimeSpec& b );



} // namespace filesystem
} // namespace openbook










#endif // TIMESPEC_H_

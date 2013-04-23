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
 *  @file   src/TimeVal.h
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_TIMEVAL_H_
#define OPENBOOK_TIMEVAL_H_

#include <sys/time.h>

namespace   openbook {
namespace filesystem {

/// just a c timeval
class TimeVal
{
    private:
        timeval m_tv;

    public:
        TimeVal( int sec=0, int usec=0 );
        operator timeval&();
        operator const timeval&() const;
        timeval* ptr();

        long int& sec();
        const long int& sec() const;
        long int& usec();
        const long int& usec() const;

        TimeVal& operator+=( const TimeVal& other );
        TimeVal& operator-=( const TimeVal& other );
};


TimeVal operator+( const TimeVal& a, const TimeVal& b );
TimeVal operator-( const TimeVal& a, const TimeVal& b );




} // namespace filesystem
} // namespace openbook










#endif // TIMEVAL_H_

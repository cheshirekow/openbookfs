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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/TimeVal.cpp
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



} // namespace filesystem
} // namespace openbook




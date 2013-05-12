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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/VersionVector.h
 *
 *  @date   May 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_VERSIONVECTOR_H_
#define OPENBOOK_FS_VERSIONVECTOR_H_

#include <map>
#include <set>
#include <cstdint>
#include <ostream>

namespace   openbook {
namespace filesystem {

/// maps client id to version number
class VersionVector:
    public std::map<int64_t,int64_t>
{
    public:
        typedef std::pair<int64_t,int64_t>  pair_t;
        typedef std::map<int64_t,int64_t>   map_t;
        typedef std::set<int64_t>           set_t;

        void keyUnion( set_t& keys, const VersionVector& other ) const;

        int64_t  operator[]( int64_t key ) const;
        int64_t& operator[]( int64_t key );

        /// for ordering of version vectors
        bool operator<( const VersionVector& other ) const;

        /// for ordering of version vectors
        bool operator<=( const VersionVector& other ) const;

        /// for ordering of version vectors
        bool operator>( const VersionVector& other ) const;

        /// for ordering of version vectors
        bool operator>=( const VersionVector& other ) const;
};

std::ostream& operator<<( std::ostream& out, const VersionVector& v );



} //< namespace filesystem
} //< namespace openbook








#endif // VERSIONVECTOR_H_

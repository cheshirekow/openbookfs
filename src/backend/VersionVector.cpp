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

#include "VersionVector.h"
#include <boost/format.hpp>

namespace   openbook {
namespace filesystem {

void VersionVector::keyUnion( set_t& keys, const VersionVector& other ) const
{
    for( auto& pair : *this )
        keys.insert( pair.first );
    for( auto& pair : other )
        keys.insert( pair.first );
}

int64_t VersionVector::operator[]( int64_t key ) const
{
    auto it = find(key);
    if( it != end() )
        return it->second;
    else
        return 0;
}

int64_t& VersionVector::operator[]( int64_t key )
{
    auto it = find(key);
    if( it != end() )
        return it->second;
    else
    {
        std::pair< map_t::iterator,bool > result =
            insert( pair_t(key,0) );
        return result.first->second;
    }
}

bool VersionVector::operator<( const VersionVector& other ) const
{
    // build a set of all keys in both vectors
    set_t   keys;
    keyUnion(keys,other);

    bool atLeastOneLess = false;

    for( auto& key : keys )
    {
        int64_t v1 = (*this)[key];
        int64_t v2 = other[key];
        if( v1 > v2 )
            return false;
        else if( v1 < v2 )
            atLeastOneLess = true;
    }

    return atLeastOneLess;
}

bool VersionVector::operator<=( const VersionVector& other ) const
{
    // build a set of all keys in both vectors
    set_t   keys;
    keyUnion(keys,other);

    for( auto& key : keys )
    {
        int64_t v1 = (*this)[key];
        int64_t v2 = other[key];
        if( v1 > v2 )
            return false;
    }

    return true;
}

bool VersionVector::operator>( const VersionVector& other ) const
{
    return (other < *this);
}

bool VersionVector::operator>=( const VersionVector& other ) const
{
    return (other <= *this);
}


std::ostream& operator<<( std::ostream& out, const VersionVector& v )
{
    out << "[ ";
    for( auto& pair : v )
        out << boost::format("(%d:%d) ") % pair.first % pair.second;
    out << "]";

    return out;
}


} //< namespace filesystem
} //< namespace openbook






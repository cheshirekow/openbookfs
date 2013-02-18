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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/ExtendedAttributes.h
 *
 *  @date   Feb 18, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_EXTENDEDATTRIBUTES_H_
#define OPENBOOK_EXTENDEDATTRIBUTES_H_

#include <string>


namespace   openbook {
namespace filesystem {

struct CStr
{
    const char*  ptr;
    unsigned int size;
};


/// extended attributes used for openbookfs meta data
namespace      xattr {


/// numeric constant which maps to a std::string used as the xattr key
enum Key
{
    STATE,          ///< file state
    VERSION,        ///< last synchronized official version
    INVALID_KEY,    ///< invalid key
    NUM_KEYS,
};

/// numeric constant which maps to a std::string used for the STATE xattr
enum State
{
    SYNCED,         ///< the file is synchronized
    DIRTY,          ///< the file has been changed by the user
    OFFICIAL,
    STALE,          ///< the file has been changed on the server but the client
    DOWNLOADING,
    CONFLICT,       ///< the file has irreconcilable changes
    CLOBBER,
    INVALID_STATE,    ///< invalid state
    NUM_STATES
};

CStr toStr( Key );
CStr toStr( State );

Key   strToKey( const std::string& );
State strToState( const std::string& );



} // namespace xattr
} // namespace filesystem
} // namespace openbook















#endif // EXTENDEDATTRIBUTES_H_

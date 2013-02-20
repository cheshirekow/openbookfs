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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/ExtendedAttributes.cpp
 *
 *  @date   Feb 18, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "ExtendedAttributes.h"

#define DEFSTR(x) { x, sizeof(x) }


namespace   openbook {
namespace filesystem {
namespace     client {

namespace      xattr {

CStr g_keyStr[NUM_KEYS] =
{
    DEFSTR("user.obfs_state"),
    DEFSTR("user.obfs_version"),
    DEFSTR("INVALID_KEY")
};

CStr g_stateStr[NUM_STATES] =
{
    DEFSTR("synced"),
    DEFSTR("dirty"),
    DEFSTR("official"),
    DEFSTR("stale"),
    DEFSTR("downloading"),
    DEFSTR("conflict"),
    DEFSTR("clobber"),
    DEFSTR("INVALID_STATE"),
};

CStr toStr( Key key )
{
    if( key >= NUM_KEYS )
        key = INVALID_KEY;
    return g_keyStr[key];
}

CStr toStr( State state )
{
    if( state >= NUM_STATES )
        state = INVALID_STATE;

    return g_stateStr[state];
}

Key strToKey( const std::string& str )
{
    for(int i=0; i < INVALID_KEY; i++)
        if( str.compare( g_keyStr[i].ptr ) == 0 )
            return (Key) i;

    return INVALID_KEY;
}

State strToState( const std::string& str )
{
    for(int i=0; i < INVALID_STATE; i++)
        if( str.compare( g_stateStr[i].ptr ) == 0 )
            return (State) i;

    return INVALID_STATE;
}





} // namespace xattr
} // namespace client
} // namespace filesystem
} // namespace openbook


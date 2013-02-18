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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/MetaFile.h
 *
 *  @date   Feb 18, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_METAFILE_H_
#define OPENBOOK_METAFILE_H_

#include <stdint.h>
#include "ExtendedAttributes.h"

namespace   openbook {
namespace filesystem {

/// private stuff for meta data file
namespace       meta {


/// numeric constant which maps to a std::string used for the STATE xattr
enum State
{
    SYNCED,     ///< the file is synchronized
    DIRTY,      ///< the file has been changed by the user
    STALE,      ///< the file has been changed on the server but the client
    CONFLICT,   ///< the file has irreconcilable changes
    NUM_STATES
};

/// meta data file structure
struct Data
{
    uint64_t    baseVersion;      ///< last synchronized version
    uint64_t    clientVersion;    ///< number of changes on client
    State       state;            ///< state of the file
    int         openFd;           ///< if the file is open, this is the fd

    static Data* map( int fd )
    {
        void* ptr = mmap( 0, sizeof(Data), PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0 );
        if( ptr == MAP_FAILED )
            return 0;
        else
            return static_cast<Data*>( ptr );
    }

    void unmap()
    {
        munmap(this,sizeof(Data));
    }

    void init()
    {
        baseVersion   = 0;
        clientVersion = 0;
        state         = DIRTY;
        openFd        = 0;
    }
};




} // namespace meta
} // namespace filesystem
} // namespace openbook



namespace   openbook {
namespace filesystem {

typedef meta::Data MetaData;

} // namespace filesystem
} // namespace openbook











#endif // METAFILE_H_

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

#include <cerrno>
#include <vector>
#include <set>

#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "ExceptionStream.h"


namespace   openbook {
namespace filesystem {
 namespace    server {

/// private stuff for meta data file
namespace       meta {


/// numeric constant which maps to a std::string used for the STATE xattr
enum State
{
    SYNCED,         ///< the file is synchronized
    INCOMPLETE,     ///< the file has not completely downloaded yet from some
                    ///  client
};

/// mmapp-able structure of a meta file
struct File
{
    uint64_t        baseVersion;      ///< official version number
    uint64_t        clientVersion;    ///< version from client if downloading
    uint64_t        owner;            ///< client who created the file
    State           state;            ///< state of the file
    unsigned int    nSubscribed;      ///< number of subscriptions
    uint64_t        subs;             ///< subscription array, first element
};

/// meta data file structure, note: locks the file until destroyed
struct Data
{
    std::string     m_path;             ///< path to the meta file
    uint64_t        m_baseVersion;      ///< official version number
    uint64_t        m_clientVersion;    ///< if downloading, client version
    uint64_t        m_owner;            ///< client who created the version
    State           m_state;            ///< state of the file
    int             m_fd;               ///< file descriptor
    std::set<uint64_t> subs;       ///< subscription array

    Data(const std::string& path );

    /// writes the initial meta file
    void create();

    /// locks and loads the file
    void load();

    /// releases and flushes the meta data
    void flush();

    uint64_t    baseVersion()   { return m_baseVersion;   }
    uint64_t    clientVersion() { return m_clientVersion; }
    uint64_t    owner()         { return m_owner;   }
    State       state()         { return m_state;   }

    void set_baseVersion    ( uint64_t version ){ m_baseVersion   = version; }
    void set_clientVersion  ( uint64_t version ){ m_clientVersion = version; }
    void set_owner          ( uint64_t owner   ){ m_owner         = owner;   }
    void set_state          ( State    state   ){ m_state         = state;   }

    void subscribe( uint64_t id );
    void unsubscribe( uint64_t id );

};




} // namespace meta
} // namespace server 
} // namespace filesystem
} // namespace openbook



namespace   openbook {
namespace filesystem {
 namespace    server {

typedef meta::Data MetaData;

} // namespace server 
} // namespace filesystem
} // namespace openbook











#endif // METAFILE_H_

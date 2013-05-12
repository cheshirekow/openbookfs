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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/jobs/SendFile.h
 *
 *  @date   May 12, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_SENDFILE_H_
#define OPENBOOK_FS_SENDFILE_H_

#include <boost/filesystem.hpp>

#include "LongJob.h"
#include "VersionVector.h"

namespace   openbook {
namespace filesystem {

class Backend;

} //< filesystem
} //< openbook



namespace   openbook {
namespace filesystem {
namespace       jobs {


/// navigates the entire directory structure and sends version information
/// for all subscribed files (including directories)
class SendFile:
    public LongJob
{
    public:
        typedef boost::filesystem::path path_t;

    private:
        Backend*        m_backend;  ///< the backend object
        int             m_peerId;   ///< the peer to send to
        path_t          m_path;     ///< path to the file
        int64_t         m_tx;
        int64_t         m_off;
        VersionVector   m_version;

    public:
        SendFile(Backend* backend, int peerId,
                    const std::string& path,
                    int64_t tx,
                    int64_t off,
                    const VersionVector& v):
            m_backend(backend),
            m_peerId(peerId),
            m_path(path),
            m_tx(tx),
            m_off(off),
            m_version(v)
        {}

        virtual ~SendFile(){}

        /// navigates the entire file system and sends version information
        /// to the connected peer
        virtual void go();


};


} //< jobs
} //< filesystem
} //< openbook



#endif // SENDFILE_H_

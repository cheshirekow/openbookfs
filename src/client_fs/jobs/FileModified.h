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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/jobs/FileModified.h
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FILEMODIFIED_H_
#define OPENBOOK_FILEMODIFIED_H_

#include "ClientJob.h"

namespace   openbook {
namespace filesystem {
namespace       jobs {

/// base class for jobs
class FileModified:
    public ClientJob
{
    private:
        std::string  m_path;
        uint64_t     m_baseVersion;
        uint64_t     m_clientVersion;

    public:
        /// simply sets the client handler so we know who to report to
        /// when the job is done
        FileModified(
            ServerHandler*        sink,
            Client*               client,
            const std::string&    path,
            uint64_t              baseVersion,
            uint64_t              clientVersion);

        /// jobs have a v-table
        virtual ~FileModified(){}

        /// there is no real job, just a message sent to the server
        virtual void doClientJob( Client* );

        /// send a message to the server that the file has been modified
        virtual void sendMessage( MessageBuffer& msg );
};



} // namespace jobs
} // namespace filesystem
} // namespace openbook


















#endif // FILEMODIFIED_H_

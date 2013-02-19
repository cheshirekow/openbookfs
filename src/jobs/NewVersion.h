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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/jobs/NewVersion.h
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_NEWVERSION_H_
#define OPENBOOK_NEWVERSION_H_


#include "jobs.h"
#include "Job.h"

namespace   openbook {
namespace filesystem {
namespace       jobs {

/// sent by the client when a file is closed after update
class NewClientVersion:
    public Job
{
    private:
        std::string m_path;
        uint64_t    m_baseVersion;
        uint64_t    m_clientVersion;

    public:
        NewClientVersion(
                unsigned int handlerVersion,
                JobSink* handler,
                const std::string& path,
                uint64_t baseVersion,
                uint64_t clientVersion):
            Job(NEW_CLIENT_VERSION,0,handlerVersion,handler),
            m_path(path),
            m_baseVersion(version),
            m_clientVersion(version)
        {}

        /// the server opens the files meta data, determines if we should
        /// download this version, and if so, starts the download
        virtual void doJob()
        {

        }
};

} // namespace jobs
} // namespace filesystem
} // namespace openbook



#endif // NEWVERSION_H_

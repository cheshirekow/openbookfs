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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/jobs/NewVersion.cpp
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "jobs/NewVersion.h"

namespace   openbook {
namespace filesystem {
namespace       jobs {


NewVersion::NewVersion(
        ClientHandler*        sink,
        unsigned int          sinkVersion,
        Server*               server,
        messages::NewVersion* msg )
:
    Job(sink,sinkVersion),
    m_server(server)
{
    m_job_id        = msg->job_id();
    m_path          = msg->path();
    m_baseVersion   = msg->base_version();
    m_clientVersion = msg->client_version();
}

void NewVersion::doJob()
{
    std::cout << "Received a NewVersion message but "
                 "implementation is incomplete" << std::endl;
}

void NewVersion::sendMessage( int fd[2], MessageBuffer& msg )
{

}


} // namespace jobs
} // namespace filesystem
} // namespace openbook

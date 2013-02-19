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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/jobs/FileModified.cpp
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "jobs/FileModified.h"

namespace   openbook {
namespace filesystem {
namespace       jobs {


FileModified::FileModified(
            ServerHandler*        sink,
            Client*               client,
            const std::string&    path,
            uint64_t              baseVersion,
            uint64_t              clientVersion )
:
    Job(sink,0),
    m_client(client),
    m_path(path),
    m_baseVersion(baseVersion),
    m_clientVersion(clientVersion)
{

}

void FileModified::doJob()
{
    std::cout << "FileModified, NO-OP:"
              << "\n    path: " << m_path
              << "\n    base: " << m_baseVersion
              << "\n  client: " << m_clientVersion
              << std::endl;
}

void FileModified::sendMessage(int fd[2], MessageBuffer& msg)
{
    std::cout << "Sending a 'New Version' message for created file "
              << m_path
              << std::endl;
    messages::NewVersion* theMsg =
            static_cast<messages::NewVersion*>( msg[MSG_NEW_VERSION] );
    theMsg->set_job_id( m_client->nextId() );
    theMsg->set_path(m_path);
    theMsg->set_base_version(m_baseVersion);
    theMsg->set_client_version(m_clientVersion);

    msg.writeEnc(fd,MSG_NEW_VERSION);
}


} // namespace jobs
} // namespace filesystem
} // namespace openbook

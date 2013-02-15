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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Job.cpp
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "Job.h"
#include "ClientHandler.h"

namespace   openbook {
namespace filesystem {

Job::Job( JobClass derived,
            unsigned int id,
            unsigned int version,
            ClientHandler* handler ):
    m_derived(derived),
    m_id(id),
    m_clientVersion(version),
    m_handler(handler)
{}


void Job::finish()
{
    /// m_handler->jobFinished(this);
}

unsigned int Job::id() const
{
    return m_id;
}

unsigned int Job::version() const
{
    return m_clientVersion;
}

JobClass Job::derived() const
{
    return m_derived;
}


} // namespace filesystem
} // namespace openbook

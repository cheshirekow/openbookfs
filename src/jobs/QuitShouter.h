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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/jobs/QuitShouter.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_QUITSHOUTER_H_
#define OPENBOOK_QUITSHOUTER_H_

#include "Job.h"

namespace   openbook {
namespace filesystem {

namespace       jobs {

/// pumped into the finished queue by the listener when the client disconnects,
/// causes the shouter to extract a job when no job is finished and kill
/// itself
/**
 *  Since this job bypasses the normal job pool there is no need to store
 *  the sink or it's version
 */
class QuitShouter:
    public Job
{
    public:
        QuitShouter():
            Job(0,0)
        {}

        virtual void doJob(){}

        virtual void sendMessage( int fd[2], MessageBuffer& msg )
        {
            throw QuitException();
        }
};

} // namespace jobs
} // namespace filesystem
} // namespace openbook






#endif // QUITSHOUTER_H_

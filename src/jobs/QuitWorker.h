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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/jobs/QuitWorker.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_QUITWORKER_H_
#define OPENBOOK_QUITWORKER_H_

#include "jobs.h"
#include "Job.h"

namespace   openbook {
namespace filesystem {

class ClientHandler;

namespace       jobs {

/// pumped into the queue by the main thread after all the network
/// threads have quit, forcing all the job workers to quit
class QuitWorker:
    public Job
{
    public:
        QuitWorker():
            Job(QUIT_WORKER,0,0)
        {}

        virtual void doJob()
        {
            throw QuitException();
        }

        virtual void sendMessage( MessageBuffer& msg ){}
};

} // namespace jobs
} // namespace filesystem
} // namespace openbook






#endif // QUITWORKER_H_

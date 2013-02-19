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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/ClientJob.h
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_CLIENTJOB_H_
#define OPENBOOK_CLIENTJOB_H_

#include "Job.h"
#include "Client.h"
#include "ServerHandler.h"

namespace   openbook {
namespace filesystem {

/// base class for jobs
class ClientJob:
    public Job
{
    private:
        Client*  m_client;  ///< stuff needed for the job

    public:
        /// simply sets the client handler so we know who to report to
        /// when the job is done
        ClientJob(
            ServerHandler*  sink,
            Client*         client);

        /// jobs have a v-table
        virtual ~ClientJob(){}

        /// do the actual job
        virtual void doClientJob( Client* )=0;

        /// does the actual job, modifies state rather than returning
        /// anything
        virtual void doJob();
};




} // namespace filesystem
} // namespace openbook














#endif // CLIENTJOB_H_

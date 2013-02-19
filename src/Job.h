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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Job.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_JOB_H_
#define OPENBOOK_JOB_H_

#include "jobs.h"
#include "JobSink.h"
#include "MessageBuffer.h"

namespace   openbook {
namespace filesystem {

/// base class for jobs
class Job
{
    protected:
        JobClass       m_derived;       ///< enum specifying derived class
        unsigned int   m_clientVersion; ///< the version of the client when
                                        ///  they created the job
        JobSink* m_sink;   ///< who to send the job to when done

    public:
        /// simply sets the client handler so we know who to report to
        /// when the job is done
        Job( JobClass derived,
                unsigned int version,
                JobSink* sink );

        /// jobs have a v-table
        virtual ~Job(){}

        /// does the actual job, modifies state rather than returning
        /// anything
        virtual void doJob()=0;

        /// returns the job to the client's queue of finished jobs
        void finish();

        /// return the client version
        unsigned int version() const;

        /// return the derived class
        JobClass derived() const;

        /// send message associated with this job, it may be an ACK or it
        /// may be an RPC
        virtual void sendMessage( MessageBuffer& buf )=0;

};




} // namespace filesystem
} // namespace openbook





#endif // JOB_H_

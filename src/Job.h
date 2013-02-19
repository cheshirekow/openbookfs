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

#include "JobSink.h"
#include "MessageBuffer.h"

namespace   openbook {
namespace filesystem {

/// thrown by special jobs to indicate that it's time to quit
class QuitException:
    public std::exception
{
    virtual const char* what() const throw()
        { return "Quitting time!"; }
};


/// base class for jobs
class Job
{
    protected:
        JobSink* m_sink; ///< who to sent the job to when done
        unsigned int   m_sinkVersion;   ///< the version of the sink when
                                        ///  they created the job

    public:
        /// simply sets the client handler so we know who to report to
        /// when the job is done
        Job( JobSink* sink = 0,
             unsigned int version=0 );

        /// jobs have a v-table
        virtual ~Job(){}

        /// does the actual job, modifies state rather than returning
        /// anything
        virtual void doJob()=0;

        /// returns the job to the sinks's queue of finished jobs, or if there
        /// is no sink, calls delete this
        void finish();

        /// return the sink version
        unsigned int version() const;

        /// send message associated with this job, it may be an ACK or it
        /// may be an RPC
        virtual void sendMessage( MessageBuffer& buf )=0;

};




} // namespace filesystem
} // namespace openbook





#endif // JOB_H_

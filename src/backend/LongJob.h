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
 *  @file   src/backend/LongJob.h
 *
 *  @date   Apr 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_LONGJOB_H_
#define OPENBOOK_FS_LONGJOB_H_

#include <exception>
#include <cpp-pthreads.h>
#include "ReferenceCounted.h"
#include "Queue.h"


namespace   openbook {
namespace filesystem {

/// exception thrown by a job meant to kill the worker
struct JobQuitException:
    public std::exception
{
    virtual ~JobQuitException() throw() {}

    virtual const char* what()
    {
        return "Job worker sutdown signalled";
    }
};

/// interface for long job objects
class LongJob:
    public ReferenceCounted
{
    private:
        RefPtr<LongJob> next;   ///< for use only by JobWorker
        friend class JobWorker;

    public:
        /// needs a v-table
        virtual ~LongJob(){}

        /// do the job
        virtual void go()=0;
};

/// special job which simply signals a shutdown for the worker
struct JobKiller:
    public LongJob
{
    virtual ~JobKiller(){}
    virtual void go(){ throw JobQuitException(); }
};


/// maintains a queue of long jobs and has a thread which continuously
/// works on that queue
class JobWorker
{
    public:
        typedef RefPtr<LongJob>    JobPtr_t;

    private:
        pthreads::Mutex     m_mutex;
        pthreads::Condition m_cond;
        JobPtr_t            m_first;
        JobPtr_t            m_last;

    public:
        JobWorker();
        ~JobWorker();

        /// add a job to the queue
        void enqueue( JobPtr_t job );

        /// pthread-callable function
        static void* dispatch_main( void* vp_worker );

    private:
        void main();
};


} //< namespace filesystem
} //< namespace openbook
















#endif // LONGJOB_H_

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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/JobHandler.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_JOBHANDLER_H_
#define OPENBOOK_JOBHANDLER_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <cpp-pthreads.h>

#include "jobs.h"
#include "ExceptionStream.h"
#include "Job.h"
#include "Pool.h"
#include "Queue.h"



namespace   openbook {
namespace filesystem {



class JobException :
    public std::runtime_error
{
    public:
        JobException( const std::string& msg ) throw():
            std::runtime_error(msg)
        {}

        virtual ~JobException() throw(){}
};



class JobHandler
{
    public:
        typedef ExceptionStream<JobException> ex;
        typedef Pool<JobHandler>              Pool_t;
        typedef Queue<Job*>                   JobQueue_t;

    private:
        Pool_t*             m_pool;             ///< pool to which this belongs
        JobQueue_t*         m_jobQueue;         ///< global job queue
        pthreads::Thread    m_thread;           ///< the thread we're running in
        pthreads::Mutex     m_mutex;            ///< locks this data


        /// main method of the job handler, waits for jobs in the queue and
        /// then does them
        void* main();

        /// static method for pthreads, calls main()
        static void* dispatch_main( void* vp_h );

    public:
        JobHandler();
        ~JobHandler();

        /// set the parent pointer and start DH parameter generation in
        /// detached thread
        void init( Pool_t*, JobQueue_t* );

        /// start the worker thread
        void start();

};



} // namespace filesystem
} // namespace openbook


#endif // JOBHANDLER_H_

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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/JobHandler.cpp
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "JobHandler.h"




namespace   openbook {
namespace filesystem {

void* JobHandler::dispatch_main( void* vp_h )
{
    JobHandler* h = static_cast<JobHandler*>(vp_h);
    return h->main();
}

void* JobHandler::main()
{
    std::cout << "Job handler " << (void*)this << " starting up\n";

    Job* job = 0;

    while(1)
    {
        // wait for a new job
        m_jobQueue->extract(job);

        // check to see if it's a terminate job
        if( job->derived() == jobs::QUIT_WORKER )
        {
            std::cout << "Job handler " << (void*)this << " received a "
                         "QUIT_WORKER job, so quitting\n";
            delete job;
            break;
        }

        // otherwise do the job
        job->doJob();

        // then send the job to the owning client's finished queue
        job->finish();
    }

    m_pool->reassign(this);
    return 0;
}

JobHandler::JobHandler()
{
    m_mutex.init();
}

JobHandler::~JobHandler()
{
    m_mutex.destroy();
}

void JobHandler::init( Pool_t* pool, JobQueue_t* queue )
{
    m_pool     = pool;
    m_jobQueue = queue;
}

void JobHandler::start()
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);

        Attr<Thread> attr;
        attr.init();
        attr << DETACHED;
        int result = m_thread.launch(attr,dispatch_main,this);
        attr.destroy();

        if( result )
        {
            std::cerr << "Failed to start job handler thread, errno " << result
                      << " : " << strerror(result) << std::endl;
            m_pool->reassign(this);
        }
    }
}



} // namespace filesystem
} // namespace openbook

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
 *  @file   src/backend/LongJob.cpp
 *
 *  @date   Apr 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <iostream>
#include "LongJob.h"


namespace   openbook {
namespace filesystem {

JobWorker::JobWorker()
{
    m_mutex.init();
    m_cond.init();
}

JobWorker::~JobWorker()
{
    m_mutex.destroy();
    m_cond.destroy();
}

void JobWorker::enqueue( JobPtr_t job )
{
    pthreads::ScopedLock lock(m_mutex);
    std::cout << "JobWorker: Enqueing job\n";

    if( m_last )
        m_last->next = job;
    else
        m_first = job;
    m_last = job;

    m_cond.signal();
}

void* JobWorker::dispatch_main( void* vp_worker )
{
    static_cast<JobWorker*>(vp_worker)->main();
    return vp_worker;
}

void JobWorker::main()
{
    while(true)
    {
        JobPtr_t job;

        // critical section lock scope
        {
            pthreads::ScopedLock lock(m_mutex);

            // wait until there is something to do
            while(!m_first)
            {
                std::cout << "JobWorker: No jobs, waiting\n";
                m_cond.wait(m_mutex);
            }

            // now that m_first is not null we can do the job
            job = m_first;

            // if this is the last job then erase both end points
            if( m_first == m_last )
            {
                m_first.clear();
                m_last.clear();
            }
            // otherwise advance first
            else
            {
                JobPtr_t next = m_first->next;
                m_first->next.clear();
                m_first = next;
            }
        // release the lock so that other threads can add jobs while
        // we're working
        }

        try
        {
            // do the job
            std::cout << "JobWorker: doing a job\n";
            job->go();
        }
        catch (const JobQuitException& ex )
        {
            std::cout << "JobWorker " << (void*)this
                      << " shutting down\n";
            break;
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Job worker caught an exception: "
                      << ex.what() << "\n";
        }
    }
    std::cout << "JobWorker " << (void*)this
              << " destroying queued jobs\n";

    pthreads::ScopedLock lock(m_mutex);
    m_last.clear();
    while(m_first)
    {
        JobPtr_t next = m_first->next;
        m_first->next.clear();
        m_first.clear();
        m_first = next;
    }

    std::cout << "JobWorker " << (void*)this
              << " exiting main loop\n";
}


} //< namespace filesystem
} //< namespace openbook









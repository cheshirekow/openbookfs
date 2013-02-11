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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/RequestHandler.cpp
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <protobuf/message.h>
#include <protobuf/io/zero_copy_stream_impl.h>
#include <errno.h>
#include "RequestHandler.h"
#include "messages.h"
#include "messages.pb.h"

namespace   openbook {
namespace filesystem {


void RequestHandler::cleanup()
{
    close(m_sock);
    // when finished we put ourselves back in the thread pool
    std::cout << "Handler " << (void*) this << " is returning to the pool"
              << std::endl;
    m_pool->reassign(this);
}

RequestHandler::RequestHandler():
    m_pool(0)
{

}

RequestHandler::~RequestHandler()
{

}

void RequestHandler::init(Pool_t* pool)
{
    m_pool = pool;
}

void RequestHandler::start( int sockfd )
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);
        m_sock = sockfd;

        Attr<Thread> attr;
        attr.init();
        attr << DETACHED;
        int result = m_thread.launch(attr,this);
        attr.destroy();

        if( result )
        {
            std::cerr << "Failed to start handler thread: ";
            switch(result)
            {
                case EAGAIN:
                    std::cerr << "EAGAIN";
                    break;

                case EINVAL:
                    std::cerr << "EINVAL";
                    break;

                case EPERM:
                    std::cerr << "EPERM";
                    break;

                default:
                    std::cerr << "unexpected errno";
                    break;
            }
            std::cerr << std::endl;
            m_pool->reassign(this);
        }
    }
}

void* RequestHandler::operator()()
{
    using namespace pthreads;
    ScopedLock lock(m_mutex);

    int received = 0;

    //create message buffers
    std::cerr << "handler " << (void*) this << " is starting up"
              << std::endl;

    try
    {

    while(1)
    {
        int msgBytes = m_msg.read(m_sock);
        m_protocol.dispatch(&m_msg);
    }

    }
    catch ( std::exception& ex )
    {
        std::cerr << ex.what() << std::endl;
        cleanup();
        return 0;
    }

    return 0;
}











} // namespace filesystem
} // namespace openbook

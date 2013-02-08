/*
 *  Copyright (C) 2012 Josh Bialkowski (jbialk@mit.edu)
 *
 *  This file is part of cppfreetype.
 *
 *  cppfreetype is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  cppfreetype is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gltk.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Handler.cpp
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "Handler.h"
#include <protobuf/message.h>
#include <protobuf/wire_format.h>

namespace   openbook {
namespace filesystem {

Handler::Handler():
    m_pool(0)
{
}

void Handler::init(HandlerPool* pool)
{
    m_pool = pool;
}

void Handler::start( int sockfd )
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);
        m_sock = sockfd;

        Attr<Thread> attr; attr << DETACHED;
        m_thread.launch(attr,this);
    }
}

void* Handler::operator()()
{
    using namespace pthreads;
    ScopedLock lock(m_mutex);

    int received = 0;

    //create protobuf object
    google::protobuf::ClientDetails client;

    while(1)
    {
        //the first bytes of the message are the varint length of the message
        unsigned int length     = 0;
        int          recv_bytes = 0;
        char         bite;

        // read the first byte
        received=recv(m_sock, &bite, 1, 0);
        if(received<0)
            return 0;

        recv_bytes += received;
        length      = (bite & 0x7f);

        // all bytes in the varint have the first bit set
        while(bite & 0x80)
        {
            // clear out the bite
            memset(&bite, 0, 1);

            // read in another byte
            received    = recv(m_sock, &bite, 1, 0);

            if(received < 0)
                return 0;

            recv_bytes += received;

            // the remaining 7bits are actual data so OR it into our
            // length binary buffer
            length |= (bite & 0x7F) << (7*(recv_bytes-1));
        }

        if( length > sm_bufsize )
        {
            std::cerr << "Received a message with size " << length <<
                         " whereas my buffer is only size " << sm_bufsize;
        }

        // now we can read in the rest of the message
        // since we know it's length
        //receive remainder of message
        recv_bytes = 0;
        while(recv_bytes < length)
        {
            received = recv(m_sock, m_buf + (sizeof(char)*recv_bytes),
                                length-recv_bytes, 0);
            if( received < 0 )
                return 0;
        }

        // now we have received a complete message from the client, so we
        // need to deserialize it
        //allocate packet buffer

        //read varint delimited protobuf object in to buffer
        google::protobuf::io::ArrayInputStream arrayIn(m_buf, recv_bytes);
        google::protobuf::io::CodedInputStream codedIn(&arrayIn);
        google::protobuf::io::CodedInputStream::Limit msgLimit
                                        = codedIn.PushLimit(recv_bytes);
        client.ParseFromCodedStream(&codedIn);
        codedIn.PopLimit(msgLimit);

        //purge buffer
        free(buffer);
    }

    // when finished we put ourselves back in the thread pool
    m_pool->reassign(this);

    return 0;
}



HandlerPool::HandlerPool()
{
    m_mutex.init();
    pthreads::ScopedLock lock(m_mutex);

    m_available.reserve(sm_maxconn);
    for(int i=0; i < sm_maxconn; i++)
    {
        m_handlers[i].init(this);
        m_available.push_back(m_handlers + i);
    }
}

Handler* HandlerPool::getAvailable()
{
    Handler* h = 0;

    // lock scope
    {
        pthreads::ScopedLock lock(m_mutex);
        if( m_available.size() == 0 )
            return 0;

        h = m_available.back();
        m_available.pop_back();
    }

    return h;
}

void HandlerPool::reassign( Handler* h )
{
    pthreads::ScopedLock lock(m_mutex);
    m_available.push_back(h);
}








} // namespace filesystem
} // namespace openbook

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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/jobs/PingJob.h
 *
 *  @date   Apr 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_PINGJOB_H_
#define OPENBOOK_FS_PINGJOB_H_

#include <iostream>
#include "LongJob.h"

namespace   openbook {
namespace filesystem {
namespace       jobs {

class Ping:
    public LongJob
{
    private:
        Backend*    m_backend;  ///< the backend object
        int         m_peerId;   ///< peer to reply to

    public:
        Ping(Backend* backend, int peerId):
            m_backend(backend),
            m_peerId(peerId)
        {}

        virtual ~Ping(){}

        virtual void go()
        {
            std::cout << "PingJob: sleeping\n";
            sleep(2);
            std::cout << "PingJob: queing PONG\n";
            messages::Ping* ping = new messages::Ping();
            ping->set_payload(0xdeadf00d);
            m_backend->sendMessage(m_peerId,ping);
        }
};

class Pong:
    public LongJob
{
    private:
        Backend*    m_backend;  ///< the backend object
        int         m_peerId;   ///< peer to reply to

    public:
        Pong(Backend* backend, int peerId):
            m_backend(backend),
            m_peerId(peerId)
        {}

        virtual ~Pong(){}

        virtual void go()
        {
            std::cout << "PongJob: sleeping\n";
            sleep(2);
            std::cout << "PongJob: queing Ping\n";
            messages::Pong* pong = new messages::Pong();
            pong->set_payload(0xdeadf00d);
            m_backend->sendMessage(m_peerId,pong);
        }
};

} //< jobs
} //< filesystem
} //< openbook















#endif // PINGJOB_H_

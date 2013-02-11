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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Protocol.cpp
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "Protocol.h"
#include "messages.h"
#include <iostream>

namespace   openbook {
namespace filesystem {

Protocol::Protocol()
{
    m_state = AUTH_REQUEST;
}

Protocol::~Protocol()
{

}

void Protocol::dispatch( MessageBuffer* msg )
{
    std::cout << "Handing message of type: " << (int)msg->type() << std::endl;

    switch(m_state)
    {
        case AUTH_REQUEST:
        {
            if(msg->type() != MSG_AUTH_REQ)
                ex()() << "Protocol Error: client sent message type "
                       << messageIdToString(msg->type())
                       << " (" << (int) msg->type() << ") "
                       << "when expecting MSG_AUTH_REQ";

            std::cout << "Handling authentication request" << std::endl;
            m_state = AUTH_CHALLENGE;
            break;
        }

        case AUTH_CHALLENGE:
        {
            if(msg->type() != MSG_AUTH_SOLN)
                ex()() << "Protocol Error: client sent message type "
                       << messageIdToString(msg->type())
                       << " (" << (int) msg->type() << ") "
                       << "when expecting MSG_AUTH_SOLUTION";

            std::cout << "Handling solution response" << std::endl;
            m_state = IDLE;
            break;
        }

        case IDLE:
        {
            std::cout << "Recieved message of type: "
                      << messageIdToString(msg->type())
                      << std::endl;
            break;
        }

        default:
        {
            ex()() << "Internal Error: protocol is in an unexpected state";
            break;
        }
    }
}




} // namespace filesystem
} // namespace openbook





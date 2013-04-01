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
 *  @file   src/messages.cpp
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "messages.h"
#include "ExceptionStream.h"

namespace   openbook {
namespace filesystem {

MessageId parseMessageId( char byte )
{
    if( 0 < byte && byte < NUM_MSG )
        return (MessageId)(byte);
    else
    {
        ex()() << "Invalid message type: " << (int)byte;
        return INVALID_MESSAGE;
    }
}

const char* messageIdToString( MessageId id )
{
    const char* str[] =
    {
        "QUIT",
        "PING",
        "PONG",
        "DH_PARAMS",
        "KEY_EXCHANGE",
        "CEK",
        "AUTH_REQ",
        "AUTH_CHALLENGE",
        "AUTH_SOLUTION",
        "AUTH_RESULT",
        "SUBSCRIBE",
        "UNSUBSCRIBE",
        "NEW_VERSION",
        "REQUEST_FILE",
        "FILE_INFO",
        "FILE_CHUNK",
        "DIR_INFO",
        "DIR_CHUNK",
        "INVALID_ID"
    };

    if( 0 < id && id < NUM_MSG )
        return str[id];
    else
        return str[NUM_MSG];
}



} // namespace filesystem
} // namespace openbook


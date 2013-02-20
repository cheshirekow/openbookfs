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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/messages.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_MESSAGES_H_
#define OPENBOOK_MESSAGES_H_

#include "messages.pb.h"

namespace   openbook {
namespace filesystem {

enum MessageId
{
    MSG_QUIT=0,             ///< special message to quit
    MSG_PING,
    MSG_PONG,
    MSG_DH_PARAMS,
    MSG_KEY_EXCHANGE,
    MSG_CEK,
    MSG_AUTH_REQ,
    MSG_AUTH_CHALLENGE,
    MSG_AUTH_SOLN,
    MSG_AUTH_RESULT,
    MSG_JOB_FINISHED,
    MSG_NEW_VERSION,
    MSG_REQUEST_CHUNK,
    MSG_FILE_CHUNK,
    MSG_COMMIT,
    INVALID_MESSAGE,
    NUM_MSG = INVALID_MESSAGE,
};


const char* messageIdToString( char id );

typedef google::protobuf::Message Message;

struct TypedMessage
{
    MessageId   type;   ///< tells us how to cast the message
    Message*    msg;    ///< base class pointer to the message

    /// fill constructor with defaults
    TypedMessage( MessageId type=INVALID_MESSAGE, Message* msg=0):
        type(type),
        msg(msg)
    {}
};

/// mappes MessageId to the message type
template < MessageId ID >
struct MessageType;

template <> struct MessageType<MSG_PING>         { typedef messages::Ping         type; };
template <> struct MessageType<MSG_PONG>         { typedef messages::Pong         type; };
template <> struct MessageType<MSG_NEW_VERSION>  { typedef messages::NewVersion   type; };
template <> struct MessageType<MSG_REQUEST_CHUNK>{ typedef messages::RequestChunk type; };
template <> struct MessageType<MSG_FILE_CHUNK>   { typedef messages::FileChunk    type; };
template <> struct MessageType<MSG_COMMIT>       { typedef messages::Commit       type; };

/// upcasts a generic message pointer
template < MessageId ID >
typename MessageType<ID>::type* message_cast( Message* msg )
{
    typedef typename MessageType<ID>::type  UpType;
    return static_cast< UpType* >( msg );
}


} // namespace filesystem
} // namespace openbook



#endif // MESSAGES_H_

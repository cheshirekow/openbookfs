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
 *  @file   src/messages.h
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

/// unique numeric identifiers for each message type
enum MessageId
{
    MSG_QUIT=0,             ///< special message to quit
    MSG_PING,
    MSG_PONG,
    MSG_LEADER_ELECT,
    MSG_DH_PARAMS,
    MSG_KEY_EXCHANGE,
    MSG_CEK,
    MSG_AUTH_REQ,
    MSG_AUTH_CHALLENGE,
    MSG_AUTH_SOLN,
    MSG_AUTH_RESULT,
    MSG_SUBSCRIBE,
    MSG_UNSUBSCRIBE,
    MSG_NEW_VERSION,
    MSG_REQUEST_FILE,
    MSG_FILE_INFO,
    MSG_FILE_CHUNK,
    MSG_DIR_INFO,
    MSG_DIR_CHUNK,
    INVALID_MESSAGE,
    NUM_MSG = INVALID_MESSAGE,
};

/// parses a byte into a MessageId
MessageId parseMessageId( char byte );

/// returns a string corresponding to the specified message id
const char* messageIdToString( MessageId id );

/// all messages are subclasses of googles protocol buffer messages
typedef google::protobuf::Message Message;

/// Encapsulates a generic message pointer with it's MessageId so that it
/// can later be cast to the right type
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

/// maps MessageId to the message type
template < MessageId ID > struct MessageType;

/// maps message type to MessageId
template < typename T > struct MessageTypeToId;

/// @cond MessageTypeTemplateInstantiations
#define MAP_MSG_TYPE(ID,TYPE) \
template <> struct MessageType<MSG_##ID> { typedef messages::TYPE type; }; \
template <> struct MessageTypeToId<messages::TYPE> { static const MessageId ID = MSG_##ID; };

MAP_MSG_TYPE(PING,              Ping)
MAP_MSG_TYPE(PONG,              Pong)
MAP_MSG_TYPE(LEADER_ELECT,      LeaderElect)
MAP_MSG_TYPE(DH_PARAMS,         DiffieHellmanParams)
MAP_MSG_TYPE(KEY_EXCHANGE,      KeyExchange)
MAP_MSG_TYPE(CEK,               ContentKey)
MAP_MSG_TYPE(AUTH_REQ,          AuthRequest)
MAP_MSG_TYPE(AUTH_CHALLENGE,    AuthChallenge)
MAP_MSG_TYPE(AUTH_SOLN,         AuthSolution)
MAP_MSG_TYPE(AUTH_RESULT,       AuthResult)
MAP_MSG_TYPE(SUBSCRIBE,         Subscribe)
MAP_MSG_TYPE(UNSUBSCRIBE,       Unsubscribe)
MAP_MSG_TYPE(NEW_VERSION,       NewVersion)
MAP_MSG_TYPE(REQUEST_FILE,      RequestFile)
MAP_MSG_TYPE(FILE_INFO,         FileInfo)
MAP_MSG_TYPE(FILE_CHUNK,        FileChunk)
MAP_MSG_TYPE(DIR_INFO,          DirInfo)
MAP_MSG_TYPE(DIR_CHUNK,         DirChunk)


/// @endcond MessageTypeTemplateInstantiations

/// upcasts a generic message pointer to it's derived type
template < MessageId ID >
typename MessageType<ID>::type* message_cast( Message* msg )
{
    typedef typename MessageType<ID>::type  UpType;
    return static_cast< UpType* >( msg );
}


} // namespace filesystem
} // namespace openbook



#endif // MESSAGES_H_

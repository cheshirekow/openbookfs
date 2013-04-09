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

/// @cond MessageTypeTemplateInstantiations
template <> struct MessageType<MSG_PING>            { typedef messages::Ping            type; };
template <> struct MessageType<MSG_PONG>            { typedef messages::Pong            type; };
template <> struct MessageType<MSG_LEADER_ELECT>    { typedef messages::LeaderElect     type; };
template <> struct MessageType<MSG_DH_PARAMS>       { typedef messages::DiffieHellmanParams type; };
template <> struct MessageType<MSG_KEY_EXCHANGE>    { typedef messages::KeyExchange     type; };
template <> struct MessageType<MSG_CEK>             { typedef messages::ContentKey      type; };
template <> struct MessageType<MSG_AUTH_REQ>        { typedef messages::AuthRequest     type; };
template <> struct MessageType<MSG_AUTH_CHALLENGE>  { typedef messages::AuthChallenge   type; };
template <> struct MessageType<MSG_AUTH_SOLN>       { typedef messages::AuthSolution    type; };
template <> struct MessageType<MSG_AUTH_RESULT>     { typedef messages::AuthResult      type; };
template <> struct MessageType<MSG_SUBSCRIBE>       { typedef messages::Subscribe       type; };
template <> struct MessageType<MSG_UNSUBSCRIBE>     { typedef messages::Unsubscribe     type; };
template <> struct MessageType<MSG_NEW_VERSION>     { typedef messages::NewVersion      type; };
template <> struct MessageType<MSG_REQUEST_FILE>    { typedef messages::RequestFile     type; };
template <> struct MessageType<MSG_FILE_INFO>       { typedef messages::FileInfo        type; };
template <> struct MessageType<MSG_FILE_CHUNK>      { typedef messages::FileChunk       type; };
template <> struct MessageType<MSG_DIR_INFO>        { typedef messages::DirInfo         type; };
template <> struct MessageType<MSG_DIR_CHUNK>       { typedef messages::DirChunk        type; };

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

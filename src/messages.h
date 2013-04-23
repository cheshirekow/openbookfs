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
#include "msg_gen/MessageId.h"
#include "msg_gen/MessageMap.h"
#include "msg_gen/MessageStr.h"

namespace   openbook {
namespace filesystem {

/// parses a byte into a MessageId
MessageId parseMessageId( char byte );

/// all messages are subclasses of googles protocol buffer messages
typedef google::protobuf::Message Message;

/// Encapsulates a generic message pointer with it's MessageId so that it
/// can later be cast to the right type
struct TypedMessage
{
    MessageId   type;   ///< tells us how to cast the message
    Message*    msg;    ///< base class pointer to the message

    /// fill constructor with defaults
    TypedMessage( MessageId type=MSG_INVALID, Message* msg=0):
        type(type),
        msg(msg)
    {}
};


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

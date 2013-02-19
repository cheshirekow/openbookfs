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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/ClientMessage.h
 *
 *  @date   Feb 19, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_CLIENTMESSAGE_H_
#define OPENBOOK_CLIENTMESSAGE_H_

#include "messages.h"


namespace   openbook {
namespace filesystem {

class ClientHandler;

struct ClientMessage
{
    ClientHandler*  client;
    uint64_t        client_id;
    TypedMessage    typed;

    ClientMessage( ClientHandler* client=0, uint64_t client_id=0,
                   MessageId id=INVALID_MESSAGE, Message* msg=0 ):
       client(client),
       client_id(client_id),
       typed(id,msg)
    {}
};



} // namespace filesystem
} // namespace openbook
















#endif // CLIENTMESSAGE_H_

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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Protocol.h
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_PROTOCOL_H_
#define OPENBOOK_PROTOCOL_H_

#include "MessageBuffer.h"

namespace   openbook {
namespace filesystem {


class ProtocolException :
    public std::runtime_error
{
    public:
        ProtocolException( const std::string& msg ) throw():
            std::runtime_error(msg)
        {}

        virtual ~ProtocolException() throw(){}
};




/// manages an array of protocol handlers, each handler only accepts messages
/// which are appropriate during that phase of the protocol
class Protocol
{
    public:
        typedef ExceptionStream<ProtocolException> ex;

        enum State
        {
            AUTH_REQUEST,        ///< waiting for auth request
            AUTH_CHALLENGE,      ///< waiting for challenge response
            IDLE,                ///< waiting for file system messages
        };

    private:
        State   m_state;    ///< state of the protocol

    public:
        /// initialize handlers
        Protocol();

        /// destroys handlers
        ~Protocol();

        /// dispatch message to the current handler
        void dispatch( MessageBuffer* msg );

};



} // namespace filesystem
} // namespace openbook




#endif // PROTOCOL_H_

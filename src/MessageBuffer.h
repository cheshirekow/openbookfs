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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/MessageBuffer.h
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_MESSAGEBUFFER_H_
#define OPENBOOK_MESSAGEBUFFER_H_


#include <exception>
#include <stdexcept>
#include <string>
#include <protobuf/message.h>

#include "ExceptionStream.h"
#include "messages.h"
#include "messages.pb.h"

#include <crypto++/rsa.h>
#include <crypto++/cryptlib.h>
#include <crypto++/rng.h>
#include <crypto++/dh2.h>



namespace   openbook {
namespace filesystem {


class MessageException :
    public std::runtime_error
{
    public:
        MessageException( const std::string& msg ) throw():
            std::runtime_error(msg)
        {}

        virtual ~MessageException() throw(){}
};


/// storage for a block of data which will eventually be parsed as a
/// protocol buffer
class MessageBuffer
{
    public:
        typedef ExceptionStream<MessageException> ex;

    private:
        /// size of the static buffer used to cache data
        static const unsigned int BUFSIZE = 1024;

        std::string  m_cipher;          ///< encrypted data
        std::string  m_plain;           ///< decrypted

        // inidiviual message structures
        messages::DiffieHellmanParams   m_dhParams;
        messages::KeyExchange   m_keyExchange;
        messages::ContentKey    m_cek;
        messages::AuthRequest   m_authReq;
        messages::AuthChallenge m_authChallenge;
        messages::AuthSolution  m_authSoln;
        messages::AuthResult    m_authResult;

        /// array of generic message pointers
        google::protobuf::Message* m_msgs[NUM_MSG];


        /// throws a MessageException if value is 0
        void checkForDisconnect( int value );

    public:
        /// fills message array
        MessageBuffer();

        google::protobuf::Message* operator[]( unsigned int );

        /// read an unencrypted message
        char read( int sockfd);

        /// write an unencrypted message
        void write( int sockfd, char type);

        /// read a message from a socket, will throw a MessageException
        /// on any problems
        char read( int sockfd,
                    CryptoPP::PrivateKey& key,
                    CryptoPP::RandomNumberGenerator& rng);

        /// write a message to a socket, will throw a MessageException on
        /// any problems
        void write( int sockfd, char type,
                    CryptoPP::PublicKey& key,
                    CryptoPP::RandomNumberGenerator& rng );


};



} // namespace filesystem
} // namespace openbook



#endif // MESSAGEBUFFER_H_

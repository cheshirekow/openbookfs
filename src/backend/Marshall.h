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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/Marshall.h
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_MARSHALL_H_
#define OPENBOOK_FS_MARSHALL_H_

#include <exception>
#include <stdexcept>
#include <string>

#include <crypto++/rsa.h>
#include <crypto++/cryptlib.h>
#include <crypto++/rng.h>
#include <crypto++/osrng.h>
#include <crypto++/gcm.h>
#include <crypto++/aes.h>

#include <protobuf/message.h>

#include "messages.h"
#include "messages.pb.h"
#include "ExceptionStream.h"
#include "ReferenceCounted.h"


namespace   openbook {
namespace filesystem {


/// Encapsulates a generic message pointer with it's MessageId and a reference
struct AutoMessage :
    public ReferenceCounted
{
    MessageId   type;   ///< tells us how to cast the message
    Message*    msg;    ///< base class pointer to the message

    /// fill constructor with defaults
    AutoMessage( MessageId type=INVALID_MESSAGE, Message* msg=0):
        type(type),
        msg(msg)
    {}

    ~AutoMessage();
};




/// appends size/type headers to protocol buffer messages and reads/writes
/// them from a file descriptor with optional encryption
class Marshall
{
    public:
        typedef CryptoPP::SecByteBlock                    IV_t;
        typedef CryptoPP::GCM<CryptoPP::AES>::Decryption  Decrypt_t;
        typedef CryptoPP::GCM<CryptoPP::AES>::Encryption  Encrypt_t;

    private:
        /// size of the static buffer used to cache data
        static const unsigned int BUFSIZE = 1024;

        std::string  m_cipher;          ///< encrypted data
        std::string  m_plain;           ///< decrypted

        int         m_fd;   ///< file descriptor

        IV_t        m_iv;   ///< initial vector
        Encrypt_t   m_enc;  ///< AES encryptor
        Decrypt_t   m_dec;  ///< AES decryptor

        /// throws an exception if value is 0
        void checkForDisconnect( int value );

        /// read the header and get the number of bytes of the message
        void readSize();

        /// read n bytes from the stream into m_cipher
        void readData();

        /// decode data from cipher to plain
        void decrypt();

        /// deserialize from the specified buffer
        RefPtr<AutoMessage> deserialize( std::string& data );

        /// serialize to the specified buffer
        void serialize( RefPtr<AutoMessage> msg, std::string& data );

        /// encode data from plain to cipher
        void encrypt();

        /// write the header to the stream
        void writeSize();

        /// write the data to the stream
        void writeData();



    public:
        Marshall();

        /// set the file descriptor to read/write from
        void setFd( int fd );

        /// initializes encryptor/decryptor with content encryption key and
        /// initial vector
        void initAES( const CryptoPP::SecByteBlock& cek,
                      const CryptoPP::SecByteBlock& iv );

        /// read an encrypted message from a socket,
        /**
         *  Allocates an appropriate message and returns it in the result. An
         *  exception will be thrown on any errors. The message is allocated
         *  on the heap instead of using the internal buffers. The
         *  allocated message pointer is owned by the caller
         */
        RefPtr<AutoMessage> readEnc( );

        /// read an encrypted message from a socket,
        /**
         *  Allocates an appropriate message and returns it in the result. An
         *  exception will be thrown on any errors. The message is allocated
         *  on the heap instead of using the internal buffers. The
         *  allocated message pointer is owned by the caller
         */
        RefPtr<AutoMessage> read(  );

        /// write an encryupted message to a socket, will throw a
        /// MessageException on any problems
        /**
         *  Does not destroy the message, do the desctruction in the calling
         *  method
         */
        void writeEnc( RefPtr<AutoMessage> msg );


        /// write an encrypted message to a socket, will throw a
        /// MessageException on any problems
        /**
         *  Does not destroy the message, do the desctruction in the calling
         *  method
         */
        void write( RefPtr<AutoMessage> msg );



};



} // namespace filesystem
} // namespace openbook




#endif // MARSHALL_H_

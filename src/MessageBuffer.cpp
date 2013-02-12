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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/MessageBuffer.cpp
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

#include <protobuf/io/zero_copy_stream_impl.h>
#include <crypto++/filters.h>

#include "MessageBuffer.h"



namespace   openbook {
namespace filesystem {


MessageBuffer::MessageBuffer()
{
    m_cipher.reserve(BUFSIZE);
    m_plain.reserve(BUFSIZE);

    m_msgs[MSG_KEY_EXCHANGE]    = &m_keyExchange;
    m_msgs[MSG_AUTH_REQ]        = &m_authReq;
    m_msgs[MSG_AUTH_CHALLENGE]  = &m_authChallenge;
    m_msgs[MSG_AUTH_SOLN]       = &m_authSoln;
    m_msgs[MSG_AUTH_RESULT]     = &m_authResult;
}


google::protobuf::Message* MessageBuffer::operator[]( unsigned int i )
{
    if( i >= NUM_MSG )
        return 0;
    else
        return m_msgs[i];
}


void MessageBuffer::checkForDisconnect( int value )
{
    if( value == 0 )
        ex()() <<  "client disconnected";
}


// message format
// first 2 bytes:   size of message
// size bytes:      message
char MessageBuffer::read( int sockfd )
{
    char header[2];     //< header bytes
    int  received;      //< result of recv
    received = recv(sockfd, header, 2, 0);

    checkForDisconnect(received);
    if(received < 0)
        ex()() <<  "failed to read message header from client";

    // the first bytes of the message
    char         type;      //< message enum
    unsigned int size;      //< size of the message

    size       = header[0] ; //< the rest are size
    size      |= header[1] << 8;

    int     recv_bytes   = 0;
    char    byte;

    if( size > BUFSIZE )
        ex()() << "Received a message with size " << size <<
                     " whereas my buffer is only size " << BUFSIZE;

    std::cout << "Receiving "
              << "unencrypted message of size " << size << " bytes "
              << std::endl;

    m_plain.resize(size);

    // now we can read in the rest of the message
    // since we know it's length
    //receive remainder of message
    recv_bytes = 0;
    while(recv_bytes < size)
    {
        received = recv(sockfd, &m_plain[0] + recv_bytes,
                        size-recv_bytes, 0);
        checkForDisconnect(received);
        if(received < 0)
            ex()() <<  "failed to read byte " << recv_bytes
                    << "of the message size";
        recv_bytes += received;
    }

    std::cout << "Finished reading " << recv_bytes << "bytes" << std::endl;

    // the first decoded byte is the type
    type = m_plain[0];

    // if it's a valid type attempt to parse the message
    if( type < 0 || type >= NUM_MSG )
        ex()() << "Invalid message type: " << (int)type;

    if( !m_msgs[type]->ParseFromArray(&m_plain[1],m_plain.size()-1) )
        ex()() << "Failed to parse message";

    std::cout << "   read done\n";
    return type;
}

void MessageBuffer::write( int sockfd, char type )
{
    namespace io = google::protobuf::io;

    unsigned int msgSize  = m_msgs[type]->ByteSize();
    unsigned int typeSize = 1;
    unsigned int size     = typeSize + msgSize;
    char         header[2]= {0,0};

    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    // make sure we have storage
    m_plain.resize(size,'\0');
    m_plain[0] = type;              //< 8 bits of type

    // the serialized message
    m_msgs[type]->SerializeToArray(&m_plain[1],msgSize);

    header[0] |= size & 0xFF;   //< first byte of size
    header[1]  = size >> 8;     //< second byte of size

    std::cout << "Sending message with " << size
              << " bytes" << std::endl;

    // send header
    int sent = 0;

    sent = send( sockfd, header, 2, 0 );
    checkForDisconnect(sent);
    if( sent != 2 )
        ex()() << "failed to send header over socket";

    // send data
    sent = send( sockfd, &m_plain[0], size, 0 );
    checkForDisconnect(sent);
    if( sent != size )
        ex()() << "failed to send message over socket";

}


// message format
// first bit:       encrypted
// next  15 bits:   unsigned size (max 2^15)
// after first two bytes: message data
char MessageBuffer::read( int sockfd,
                         CryptoPP::PrivateKey& key,
                         CryptoPP::RandomNumberGenerator& rng)
{
    char header[2];     //< header bytes
    int  received;      //< result of recv
    received = recv(sockfd, header, 2, 0);

    checkForDisconnect(received);
    if(received < 0)
        ex()() <<  "failed to read message header from client";

    // the first bytes of the message
    bool         encrypt;   //< message is encrypted
    char         type;      //< message enum
    unsigned int size;      //< size of the message

    encrypt    = header[0] & 0x80; //< first bit
    size       = header[0] & 0x7f; //< the rest are size
    size      |= header[1] << 7;

    int     recv_bytes   = 0;
    char    byte;

    if( size > BUFSIZE )
        ex()() << "Received a message with size " << size <<
                     " whereas my buffer is only size " << BUFSIZE;

    std::cout << "Receiving "
              << (encrypt ? "" : "un" )
              << "encrypted message of size " << size << " bytes "
              << std::endl;

    std::string* target = encrypt ? &m_cipher : &m_plain;
    target->resize(size);

    // now we can read in the rest of the message
    // since we know it's length
    //receive remainder of message
    recv_bytes = 0;
    while(recv_bytes < size)
    {
        received = recv(sockfd, &(*target)[0] + recv_bytes,
                        size-recv_bytes, 0);
        checkForDisconnect(received);
        if(received < 0)
            ex()() <<  "failed to read byte " << recv_bytes
                    << "of the message size";
        recv_bytes += received;
    }

    std::cout << "Finished reading " << recv_bytes << "bytes" << std::endl;

    // if it's encrypted, then decrypt it
    if(encrypt)
    {
        using namespace CryptoPP;
        RSAES_OAEP_SHA_Decryptor d(key);

        StringSource ss2(m_cipher,true,
                new PK_DecryptorFilter(rng, d, new StringSink(m_plain)
            ) // PK_DecryptorFilter
        ); // StringSource
    }

    // the first decoded byte is the type
    type = m_plain[0];

    // if it's a valid type attempt to parse the message
    if( type < 0 || type >= NUM_MSG )
        ex()() << "Invalid message type: " << (int)type;

    if( !m_msgs[type]->ParseFromArray(&m_plain[1],m_plain.size()-1) )
        ex()() << "Failed to parse message";

    std::cout << "   read done\n";

    return type;
}

void MessageBuffer::write( int sockfd, char type,
                    CryptoPP::PublicKey& key,
                    CryptoPP::RandomNumberGenerator& rng )
{
    namespace io = google::protobuf::io;

    unsigned int msgSize  = m_msgs[type]->ByteSize();
    unsigned int typeSize = 1;
    unsigned int size     = typeSize + msgSize;
    char         header[2]= {0,0};

    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    // make sure we have storage
    m_plain.resize(size,'\0');
    m_plain[0] = type;              //< 8 bits of type

    // the serialized message
    m_msgs[type]->SerializeToArray(&m_plain[1],msgSize);

    std::string* target = &m_plain;

    // if encrypted
    if( type != MSG_AUTH_REQ )
    {
        // mark as encrypted
        header[0] |= 0x80;

        // encrypt message
        using namespace CryptoPP;
        RSAES_OAEP_SHA_Encryptor e(key);

        StringSource ss1(m_plain, true,
            new PK_EncryptorFilter(rng, e, new StringSink(m_cipher)
           ) // PK_EncryptorFilter
        ); // StringSource

        target = &m_cipher;
    }

    // get the size of the target string
    size = target->size();
    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    header[0] |= ( size & 0x7F );   //< first 7 bits of size
    header[1]  = size >> 7;         //< remaining 7 bits of size

    std::cout << "Sending message with " << size << ", "
              << target->size() << " bytes" << std::endl;

    // send header
    int sent = 0;

    sent = send( sockfd, header, 2, 0 );
    checkForDisconnect(sent);
    if( sent != 2 )
        ex()() << "failed to send header over socket";

    // send data
    sent = send( sockfd, &(*target)[0], size, 0 );
    checkForDisconnect(sent);
    if( sent != size )
        ex()() << "failed to send message over socket";

}



} // namespace filesystem
} // namespace openbook

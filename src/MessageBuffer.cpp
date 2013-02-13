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
#include <cerrno>

#include <protobuf/io/zero_copy_stream_impl.h>
#include <crypto++/filters.h>

#include "MessageBuffer.h"



namespace   openbook {
namespace filesystem {


MessageBuffer::MessageBuffer()
{
    m_cipher.reserve(BUFSIZE);
    m_plain.reserve(BUFSIZE);

    m_msgs[MSG_DH_PARAMS]       = &m_dhParams;
    m_msgs[MSG_KEY_EXCHANGE]    = &m_keyExchange;
    m_msgs[MSG_CEK]             = &m_cek;
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
    unsigned char header[2];     //< header bytes
    int           received;      //< result of recv
    received = recv(sockfd, header, 2, 0);

    checkForDisconnect(received);
    if(received < 0)
        ex()() <<  "failed to read message header from client";

    // the first bytes of the message
    char         type;      //< message enum
    unsigned int size;      //< size of the message

    size    = header[0] ; //< the rest are size
    size   |= header[1] << 8;

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

    std::cout << "Finished reading " << recv_bytes << " bytes" << std::endl;

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

    unsigned int  msgSize  = m_msgs[type]->ByteSize();
    unsigned int  typeSize = 1;
    unsigned int  size     = typeSize + msgSize;
    unsigned char header[2]= {0,0};

    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    // make sure we have storage
    m_plain.resize(size,'\0');
    m_plain[0] = type;              //< 8 bits of type

    // the serialized message
    m_msgs[type]->SerializeToArray(&m_plain[1],msgSize);

    header[0]  = size & 0xFF;   //< first byte of size
    header[1]  = size >> 8;     //< second byte of size

    std::cout << "Sending unencrypted message with " << size
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
                            CryptoPP::GCM<CryptoPP::AES>::Decryption& dec)
{
    unsigned char header[2];     //< header bytes
    int           received;      //< result of recv
    received = recv(sockfd, header, 2, 0);

    checkForDisconnect(received);
    if(received < 0)
        ex()() <<  "failed to read message header from client";

    // the first bytes of the message
    unsigned int size;      //< size of the message
    char         type;
    size       = header[0];         //< first byte of size
    size      |= header[1] << 8;    //< second byte of size

    int     recv_bytes   = 0;
    if( size > BUFSIZE )
        ex()() << "Received a message with size " << size
               << " (" << std::hex << (int)header[0] << " " << (int)header[1]
               << std::dec << ") whereas my buffer is only size " << BUFSIZE;

    std::cout << "Receiving encrypted message of size " << size << " bytes "
              << std::endl;
    m_cipher.resize(size);

    // now we can read in the rest of the message
    // since we know it's length
    //receive remainder of message
    recv_bytes = 0;
    while(recv_bytes < size)
    {
        received = recv(sockfd, &m_cipher[0] + recv_bytes,
                        size-recv_bytes, 0);
        checkForDisconnect(received);
        if(received < 0)
            ex()() <<  "failed to read bytes after " << recv_bytes
                    << " of the message";
        recv_bytes += received;
    }

    std::cout << "Finished reading " << recv_bytes << " bytes" << std::endl;

    // decrypt the message
    m_plain.clear();
    CryptoPP::StringSource( m_cipher, true,
        new CryptoPP::AuthenticatedDecryptionFilter(
            dec,
            new CryptoPP::StringSink( m_plain )
        )
    );

    std::cout << "Finished ecryption and message authentication" << std::endl;

    // the first decoded byte is the type
    type = m_plain[0];

    // if it's a valid type attempt to parse the message
    if( type < 0 || type >= NUM_MSG )
        ex()() << "Invalid message type: " << (int)type;

    if( !m_msgs[type]->ParseFromArray(&m_plain[1],m_plain.size()-1) )
        ex()() << "Failed to parse message " << messageIdToString(type)
               << " of size " << m_plain.size()-1;

    std::cout << "   read done\n";

    return type;
}

void MessageBuffer::write( int sockfd, char type,
                            CryptoPP::GCM<CryptoPP::AES>::Encryption& enc )
{
    namespace io = google::protobuf::io;

    unsigned int  msgSize  = m_msgs[type]->ByteSize();
    unsigned int  typeSize = 1;
    unsigned int  size     = typeSize + msgSize;
    unsigned char header[2]= {0,0};

    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    // make sure we have storage
    m_plain.resize(size,'\0');
    m_plain[0] = type;              //< 8 bits of type

    // the serialized message
    m_msgs[type]->SerializeToArray(&m_plain[1],msgSize);

    // encrypt the message
    m_cipher.clear();
    CryptoPP::StringSource( m_plain, true,
        new CryptoPP::AuthenticatedEncryptionFilter(
            enc,
            new CryptoPP::StringSink( m_cipher )
        )
    );

    // get the size of the target string
    size = m_cipher.size();
    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    header[0] = size & 0xFF;   //< first byte of size
    header[1] = size >> 8;     //< second byte of size

    std::cout << "Sending message with " << msgSize << ", ("
              << size << " encrypted) bytes, header: "
              << std::hex << (int)header[0] << " " << (int)header[1] << std::dec
              << ", type: " << messageIdToString(type)
              << " (" << (int)type << ")"
              << std::endl;

    // send header
    int sent = 0;

    sent = send( sockfd, header, 2, 0 );
    checkForDisconnect(sent);
    if( sent != 2 )
        ex()() << "failed to send header over socket";

    // send data
    sent = send( sockfd, &m_cipher[0], size, 0 );
    checkForDisconnect(sent);
    if( sent != size )
        ex()() << "failed to send message over socket";

}









// message format
// first 2 bytes:   size of message
// size bytes:      message
char MessageBuffer::read( SelectSet& fd )
{
    unsigned char header[2];        //< header bytes
    int           received;         //< result of recv
    int           bytes_received=0; //< total read so far

    while( bytes_received < 2 )
    {
        // attempt read
        received = recv(fd[0], header + bytes_received, 2 - bytes_received, 0);

        // if we read some bytes
        if( received > 0 )
        {
            bytes_received += received;
            continue;
        }

        // otherwise check for problems
        checkForDisconnect(received);
        if( errno != EWOULDBLOCK )
            ex()() << "Error while trying to read message header, errno "
                   << errno << " : " << strerror( errno );

        // wait for data (do it in a loop because wait may timeout)
        std::cout << "Waiting for header" << std::endl;
        while( fd.wait() == 0 );

        // if we were signalled by anything other than data ready, then
        // just bail
        if( !fd(0) )
            ex()() << "Signaled by something other than data, quitting";
    }


    // the first bytes of the message
    char         type;      //< message enum
    unsigned int size;      //< size of the message

    size    = header[0] ; //< the rest are size
    size   |= header[1] << 8;

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
    bytes_received = 0;
    while(bytes_received < size)
    {
        // try to read some data
        received = recv(fd[0], &m_plain[0] + bytes_received,
                                                    size-bytes_received, 0);

        // if we got some bytes loop around
        if( received > 0 )
        {
            bytes_received += received;
            continue;
        }

        // otherwise check for problems
        checkForDisconnect(received);
        if( errno != EWOULDBLOCK )
            ex()() <<  "failed to read bytes " << bytes_received
                    << "+ of the message";

        // wait for data
        std::cout << "Waiting for the rest of the message" << std::endl;
        while( fd.wait() == 0 );

        // if we were signalled by something other than data, then bail
        if( !fd(0) )
            ex()() << "Signalled by something other than data while waiting for "
                      "the rest of the message";
    }

    std::cout << "Finished reading " << bytes_received << " bytes" << std::endl;

    // the first decoded byte is the type
    type = m_plain[0];

    // if it's a valid type attempt to parse the message
    if( type < 0 || type >= NUM_MSG )
        ex()() << "Invalid message type: " << (int)type;

    if( !m_msgs[type]->ParseFromArray(&m_plain[1],m_plain.size()-1) )
        ex()() << "Failed to parse message " << messageIdToString(type)
               << " of size " << m_plain.size()-1;

    std::cout << "   read done\n";
    return type;
}

void MessageBuffer::write( SelectSet& fd, char type )
{
    namespace io = google::protobuf::io;

    unsigned int  msgSize  = m_msgs[type]->ByteSize();
    unsigned int  typeSize = 1;
    unsigned int  size     = typeSize + msgSize;
    unsigned char header[2]= {0,0};

    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    // make sure we have storage
    m_plain.resize(size,'\0');
    m_plain[0] = type;              //< 8 bits of type

    // the serialized message
    m_msgs[type]->SerializeToArray(&m_plain[1],msgSize);

    header[0]  = size & 0xFF;   //< first byte of size
    header[1]  = size >> 8;     //< second byte of size

    std::cout << "Sending unencrypted message with " << size
              << " bytes" << std::endl;

    // send header
    int bytes_sent = 0;
    int sent       = 0;

    // since we only send after a message is received this should never
    // block, so no need to loop over the header
    sent = send( fd[0], header, 2, 0 );
    checkForDisconnect(sent);
    if( sent != 2 )
        ex()() << "Failed to send header over socket";

    // send data
    while( bytes_sent < size )
    {
        sent = send( fd[0], &m_plain[0]+bytes_sent, size-bytes_sent, 0 );
        if( sent > 0 )
        {
            std::cout << "   sent " << sent <<  "/" << size
                      << " bytes to netstack " << std::endl;
            bytes_sent += sent;
            continue;
        }

        checkForDisconnect(sent);
        if( errno != EWOULDBLOCK )
            ex()() << "Failed to send message over socket, errno " << errno
                   << " : " << strerror(errno);

        // wait for buffer to free up
        while( fd.wait() == 0 );

        // verify that we didn't wake up by some other signal
        if( !fd(0) )
            ex()() << "Signalled by something other than buffer freeing "
                      "up, bailing";
    }

    std::cout << "Finished sending message\n";
}


// message format
// first bit:       encrypted
// next  15 bits:   unsigned size (max 2^15)
// after first two bytes: message data
char MessageBuffer::read( SelectSet& fd,
                            CryptoPP::GCM<CryptoPP::AES>::Decryption& dec)
{
    unsigned char header[2];        //< header bytes
    int           received;         //< result of recv
    int           bytes_received=0; //< total read so far

    while( bytes_received < 2 )
    {
        // attempt read
        received = recv(fd[0], header + bytes_received, 2 - bytes_received, 0);

        // if we read some bytes
        if( received > 0 )
        {
            bytes_received += received;
            continue;
        }

        // otherwise check for problems
        checkForDisconnect(received);
        if( errno != EWOULDBLOCK )
            ex()() << "Error while trying to read message header, errno "
                   << errno << " : " << strerror( errno );

        // wait for data (do it in a loop because wait may timeout)
        std::cout << "Waiting for header" << std::endl;
        while( fd.wait() == 0 );

        // if we were signalled by anything other than data ready, then
        // just bail
        if( !fd(0) )
            ex()() << "Signaled by something other than data, quitting";
    }

    // the first bytes of the message
    char         type;      //< message enum
    unsigned int size;      //< size of the message

    size    = header[0] ; //< the rest are size
    size   |= header[1] << 8;

    if( size > BUFSIZE )
        ex()() << "Received a message with size " << size
               << " (" << std::hex << (int)header[0] << " " << (int)header[1]
               << std::dec << ") whereas my buffer is only size " << BUFSIZE;

    std::cout << "Receiving encrypted message of size " << size << " bytes "
              << std::endl;
    m_plain.resize(size);

    // now we can read in the rest of the message
    // since we know it's length
    //receive remainder of message
    bytes_received = 0;
    while(bytes_received < size)
    {
        // try to read some data
        received = recv(fd[0], &m_cipher[0] + bytes_received,
                                                    size-bytes_received, 0);

        // if we got some bytes loop around
        if( received > 0 )
        {
            bytes_received += received;
            continue;
        }

        // otherwise check for problems
        checkForDisconnect(received);
        if( errno != EWOULDBLOCK )
            ex()() <<  "failed to read bytes " << bytes_received
                    << "+ of the message";

        // wait for data
        std::cout << "Waiting for the rest of the message" << std::endl;
        while( fd.wait() == 0 );

        // if we were signalled by something other than data, then bail
        if( !fd(0) )
            ex()() << "Signalled by something other than data while waiting for "
                      "the rest of the message";
    }

    std::cout << "Finished reading " << bytes_received << " bytes" << std::endl;

    // decrypt the message
    m_plain.clear();
    CryptoPP::StringSource( m_cipher, true,
        new CryptoPP::AuthenticatedDecryptionFilter(
            dec,
            new CryptoPP::StringSink( m_plain )
        )
    );

    std::cout << "Finished decryption and message authentication" << std::endl;

    // the first decoded byte is the type
    type = m_plain[0];

    // if it's a valid type attempt to parse the message
    if( type < 0 || type >= NUM_MSG )
        ex()() << "Invalid message type: " << (int)type;

    if( !m_msgs[type]->ParseFromArray(&m_plain[1],m_plain.size()-1) )
        ex()() << "Failed to parse message " << messageIdToString(type)
               << " of size " << m_plain.size()-1;

    std::cout << "   read done\n";
    return type;
}

void MessageBuffer::write( SelectSet&fd, char type,
                            CryptoPP::GCM<CryptoPP::AES>::Encryption& enc )
{
    namespace io = google::protobuf::io;

    unsigned int  msgSize  = m_msgs[type]->ByteSize();
    unsigned int  typeSize = 1;
    unsigned int  size     = typeSize + msgSize;
    unsigned char header[2]= {0,0};

    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    // make sure we have storage
    m_plain.resize(size,'\0');
    m_plain[0] = type;              //< 8 bits of type

    // the serialized message
    m_msgs[type]->SerializeToArray(&m_plain[1],msgSize);

    // encrypt the message
    m_cipher.clear();
    CryptoPP::StringSource( m_plain, true,
        new CryptoPP::AuthenticatedEncryptionFilter(
            enc,
            new CryptoPP::StringSink( m_cipher )
        )
    );

    // get the size of the target string
    size = m_cipher.size();
    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    header[0] = size & 0xFF;   //< first byte of size
    header[1] = size >> 8;     //< second byte of size

    std::cout << "Sending message with " << msgSize << ", ("
              << size << " encrypted) bytes, header: "
              << std::hex << (int)header[0] << " " << (int)header[1] << std::dec
              << ", type: " << messageIdToString(type)
              << " (" << (int)type << ")"
              << std::endl;

    // send header
    int bytes_sent = 0;
    int sent       = 0;

    // since we only send after a message is received this should never
    // block, so no need to loop over the header
    sent = send( fd[0], header, 2, 0 );
    checkForDisconnect(sent);
    if( sent != 2 )
        ex()() << "Failed to send header over socket";

    // send data
    while( bytes_sent < size )
    {
        sent = send( fd[0], &m_plain[0]+bytes_sent, size-bytes_sent, 0 );
        if( sent > 0 )
        {
            std::cout << "   sent " << sent <<  "/" << size
                      << " bytes to netstack " << std::endl;
            bytes_sent += sent;
            continue;
        }

        checkForDisconnect(sent);
        if( errno != EWOULDBLOCK )
            ex()() << "Failed to send message over socket, errno " << errno
                   << " : " << strerror(errno);

        // wait for buffer to free up
        while( fd.wait() == 0 );

        // verify that we didn't wake up by some other signal
        if( !fd(0) )
            ex()() << "Signalled by something other than buffer freeing "
                      "up, bailing";
    }

}



} // namespace filesystem
} // namespace openbook

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

#include <sys/socket.h>
#include <unistd.h>
#include "MessageBuffer.h"
#include <protobuf/io/zero_copy_stream_impl.h>
#include <iostream>




namespace   openbook {
namespace filesystem {

void MessageBuffer::checkForDisconnect( int value )
{
    if( value == 0 )
        ex()() <<  "client disconnected";
}

const char  MessageBuffer::type() const
{
    return m_type;
}

const unsigned int MessageBuffer::size() const
{
    return m_size;
}

const char* MessageBuffer::buf() const
{
    return m_buf;
}

int MessageBuffer::read( int sockfd )
{
    int received;
    received = recv(sockfd, &m_type, 1, 0);

    checkForDisconnect(received);
    if(received < 0)
        ex()() <<  "failed to read message type from client";

    std::cout << "Reading in message of type: " << (int)m_type << std::endl;


    // the first bytes of the message are the varint length of the message
            m_size       = 0;
    int     recv_bytes   = 0;
    char    byte;

    // read the first byte
    received=recv(sockfd, &byte, 1, 0);
    checkForDisconnect(received);
    if(received < 0)
        ex()() <<  "failed to first byte of message size";

    // only the last 7 bits of the byte are significant
    recv_bytes += received;
    m_size      = (byte & 0x7f);

    // if the first bit is set it means the next byte is also part of the
    // varint
    while(byte & 0x80)
    {
        // clear out the bite
        byte = 0;

        // read in another byte
        received    = recv(sockfd, &byte, 1, 0);

        checkForDisconnect(received);
        if(received < 0)
            ex()() <<  "failed to read byte " << recv_bytes
                    << "of the message size";

        recv_bytes += received;

        // the remaining 7bits are actual data so OR it into our
        // length binary buffer shifted by 7 bits
        m_size |= (byte & 0x7F) << (7*(recv_bytes-1));
    }

    if( m_size > BUFSIZE )
        ex()() << "Received a message with size " << m_size <<
                     " whereas my buffer is only size " << BUFSIZE;

    std::cout << "   size: " << m_size<< std::endl;

    // now we can read in the rest of the message
    // since we know it's length
    //receive remainder of message
    recv_bytes = 0;
    while(recv_bytes < m_size)
    {
        received = recv(sockfd, m_buf + recv_bytes, m_size-recv_bytes, 0);
        checkForDisconnect(received);
        if(received < 0)
            ex()() <<  "failed to read byte " << recv_bytes
                    << "of the message size";
        recv_bytes += received;
    }

    std::cout << "   read done\n";

    return recv_bytes;
}

int MessageBuffer::write( int sockfd, char type, google::protobuf::Message& msg )
{
    namespace io = google::protobuf::io;

    unsigned int msgSize  = msg.ByteSize();
    unsigned int sizeSize = io::CodedOutputStream::VarintSize32(msg.ByteSize());
    unsigned int typeSize = 1;
    unsigned int totSize  = typeSize + sizeSize + msgSize;

    if( totSize > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << m_size;

    google::protobuf::io::ArrayOutputStream array_out(m_buf+1,BUFSIZE);
    google::protobuf::io::CodedOutputStream out(&array_out);
    m_buf[0] = type;
    out.WriteVarint32(msgSize);

    if( !msg.SerializeToCodedStream(&out) )
        ex()() << "Failed to serialize message";

    int sent = send( sockfd, m_buf, totSize, 0 );

    checkForDisconnect(sent);
    if( sent != totSize )
        ex()() << "failed to send message over socket";

    return sent;
}



} // namespace filesystem
} // namespace openbook

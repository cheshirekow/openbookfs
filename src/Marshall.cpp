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
 *  @file   src/backend/Marshall.cpp
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>

#include <protobuf/io/zero_copy_stream_impl.h>
#include <crypto++/filters.h>

#include "global.h"
#include "Marshall.h"
#include "SelectSpec.h"


namespace   openbook {
namespace filesystem {

/// binary search for the upcast of a message, upcast it, and then destroy it
template <int NA, int NC>
struct MessageDestroyer
{
    static const int        NB  = (NA+NC)/2;
    static const MessageId  MID = (MessageId)NB;

    static void destroy( MessageId type, Message* msg )
    {
        if( type < NB )
            MessageDestroyer<NA,NB>::destroy( type, msg );
        else if( type > NB )
            MessageDestroyer<NB,NC>::destroy( type, msg );
        else
        {
            typedef typename MessageType<MID>::type DownType;
            DownType* downcast = message_cast<MID>(msg );
            if( downcast )
                delete downcast;
        }
    }
};

/// binary search for the type of a message, create and parse the message,
/// and then return a base class pointer
template <int NA, int NC>
struct MessageParser
{
    static const int        NB  = (NA+NC)/2;
    static const MessageId  MID = (MessageId)NB;

    static Message* parse( const std::string& data )
    {
        if( data[0] < NB )
            return MessageParser<NA,NB>::parse(data);
        else if( data[0] > NB )
            return MessageParser<NB,NC>::parse(data);
        else
        {
            typedef typename MessageType<MID>::type DownType;
            DownType* msg = new DownType();
            if( !msg->ParseFromArray(&data[1], data.size()-1) )
            {
                ex()() << "Failed to parse message as "
                       << messageIdToString((MessageId)NB);
            }

            return msg;
        }
    }
};


typedef MessageDestroyer<0,NUM_MSG> MsgDestroy;
typedef MessageParser<0,NUM_MSG>    MsgParse;


AutoMessage::~AutoMessage()
{
    if(msg)
        MsgDestroy::destroy(type,msg);
}



Marshall::Marshall()
{
    m_cipher.reserve(BUFSIZE);
    m_plain.reserve(BUFSIZE);
}


void Marshall::setFd(int fd)
{
    m_fd = fd;
}

void Marshall::initAES( const CryptoPP::SecByteBlock& cek,
                             const CryptoPP::SecByteBlock& iv )
{
    m_iv = iv;
    m_enc.SetKeyWithIV(  cek.BytePtr(), cek.SizeInBytes(),
                         iv.BytePtr(),  iv.SizeInBytes());
    m_dec.SetKeyWithIV(  cek.BytePtr(), cek.SizeInBytes(),
                         iv.BytePtr(),  iv.SizeInBytes());
}


void Marshall::checkForDisconnect( int value )
{
    if( value == 0 )
        ex()() <<  "client disconnected";
}

void Marshall::readSize()
{
    using namespace select_spec;
    SelectSpec select;
    select.gen()(m_fd, READ)
                (g_termNote->readFd(), READ)
                ( TimeVal(2,0) );

    unsigned char header[2];        //< header bytes
    unsigned int  size          =2; //< size of the read
    int           received;         //< result of recv
    int           bytes_received=0; //< total read so far

    while( bytes_received < size )
    {
        // attempt read
        received = recv(m_fd, header + bytes_received,
                        size-bytes_received, 0);

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
        std::cout << "Waiting for " << size - bytes_received
                  << " more header bytes\n";
        while( select.wait() == 0 );

        // if we were signalled by anything other than data ready, then
        // just bail
        if( !select.ready(m_fd,READ) )
            ex()() << "Signaled by something other than data, quitting";
    }

    size    = header[0] ;       //< first byte of size
    size   |= header[1] << 8;   //< second byte of size

    if( size > BUFSIZE )
    {
        ex()() << "Received a message with size " << size
               << " ("
               << std::hex
               << (int)header[0] << " "
               << (int)header[1]
               << std::dec
               << ") whereas my buffer is only size " << BUFSIZE;
    }

    std::cout << "Receiving message of size " << size << " bytes "
              << std::endl;
    m_cipher.resize(size);
}

void Marshall::readData(  )
{
    /// create the select spec
    using namespace select_spec;
    SelectSpec select;
    select.gen()(m_fd, READ)
                (g_termNote->readFd(), READ)
                ( TimeVal(2,0) );

    int     received;              //< result of recv
    int     bytes_received =0;     //< total read so far
    int     size           = m_cipher.size();  //< total to read
    char*   buf            = &m_cipher[0];     //< write head

    // now we can read in the rest of the message
    // since we know it's length
    //receive remainder of message
    bytes_received = 0;

    while(bytes_received < size)
    {
        // try to read some data
        received = recv(m_fd, buf+bytes_received, size-bytes_received, 0);

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
        while( select.wait() == 0 );

        // if we were signalled by something other than data, then bail
        if( !select.ready(m_fd,READ) )
            ex()() << "Signalled by something other than data while waiting for "
                      "the rest of the message";
    }

    std::cout << "Finished reading " << bytes_received << " bytes" << std::endl;
}

void Marshall::decrypt()
{
    // decrypt the message
    m_plain.clear();
    CryptoPP::StringSource( m_cipher, true,
        new CryptoPP::AuthenticatedDecryptionFilter(
            m_dec,
            new CryptoPP::StringSink( m_plain )
        )
    );

    // reset the decryptor
    m_dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

    std::cout << "Finished decryption and message authentication" << std::endl;
}

RefPtr<AutoMessage> Marshall::deserialize( std::string& data )
{
    AutoMessage* out = new AutoMessage();

    // the first decoded byte is the type
    out->type = parseMessageId( data[0] );
    out->msg  = MsgParse::parse(data);

    return RefPtr<AutoMessage>(out);
}


RefPtr<AutoMessage> Marshall::readEnc(  )
{
    readSize();
    readData();
    decrypt();
    return deserialize( m_plain );
}

RefPtr<AutoMessage> Marshall::read( bool encrypted  )
{
    readSize();
    readData();
    if( encrypted )
    {
        decrypt();
        return deserialize( m_plain );
    }
    else
        return deserialize( m_cipher );
}

void Marshall::serialize( RefPtr<AutoMessage> msg, std::string& data )
{
    unsigned int  msgSize  = msg->msg->ByteSize();
    unsigned int  typeSize = 1;
    unsigned int  size     = typeSize + msgSize;

    if( size > BUFSIZE )
        ex()() << "Attempt to send a message of size " << msgSize
               << " but my buffer is only " << BUFSIZE;

    // make sure we have storage for the data
    data.resize(size,'\0');
    data[0] = msg->type;  //< 8 bits of type

    // the serialized message
    msg->msg->SerializeToArray(&data[1],msgSize);
}

void Marshall::encrypt()
{
    // encrypt the message
    m_cipher.clear();
    CryptoPP::StringSource( m_plain, true,
        new CryptoPP::AuthenticatedEncryptionFilter(
            m_enc,
            new CryptoPP::StringSink( m_cipher )
        )
    );

    // reset the encryptor
    m_enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

    // get the size of the target string
    if( m_cipher.size() > BUFSIZE )
        ex()() << "Attempt to send a message of (encrypted) size "
               << m_cipher.size()
               << " but my buffer is only " << BUFSIZE;


}

void Marshall::writeSize()
{
    using namespace select_spec;
    SelectSpec select;
    select.gen()(m_fd, WRITE)
                (g_termNote->readFd(), READ)
                ( TimeVal(2,0) );

    unsigned char header[2]= {0,0};
    unsigned int  size     = m_cipher.size();

    header[0] = size & 0xFF;   //< first byte of size
    header[1] = size >> 8;     //< second byte of size

    // send header
        size       = 2;
    int bytes_sent = 0;
    int sent       = 0;

    while( bytes_sent < size )
    {
        sent = send( m_fd, header+bytes_sent, size-bytes_sent, 0 );
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
        while( select.wait() == 0 );

        // verify that we didn't wake up by some other signal
        if( !select.ready(m_fd,WRITE) )
            ex()() << "Signalled by something other than buffer freeing "
                      "up, bailing";
    }
}

void Marshall::writeData()
{
    using namespace select_spec;
    SelectSpec select;
    select.gen()(m_fd, WRITE)
                (g_termNote->readFd(), READ)
                ( TimeVal(2,0) );

    // send header
    char*   buf    = &m_cipher[0];
    int     size   = m_cipher.size();
    int bytes_sent = 0;
    int sent       = 0;

    while( bytes_sent < size )
    {
        sent = send( m_fd, buf+bytes_sent, size-bytes_sent, 0 );
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
        while( select.wait() == 0 );

        // verify that we didn't wake up by some other signal
        if( !select.ready(m_fd,WRITE) )
            ex()() << "Signalled by something other than buffer freeing "
                      "up, bailing";
    }
}

void Marshall::writeEnc( RefPtr<AutoMessage> msg )
{
    serialize( msg, m_plain );
    encrypt();
    writeSize();
    writeData();
}

void Marshall::write( RefPtr<AutoMessage> msg, bool encrypted )
{
    if( encrypted )
    {
        serialize( msg, m_plain );
        encrypt();
    }
    else
        serialize( msg, m_cipher );

    writeSize();
    writeData();
}




} // namespace filesystem
} // namespace openbook









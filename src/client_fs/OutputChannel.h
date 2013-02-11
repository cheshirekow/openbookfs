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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/OutputChannel.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_OUTPUTCHANNEL_H_
#define OPENBOOK_OUTPUTCHANNEL_H_

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <cpp-pthreads.h>
#include <vector>

namespace   openbook {
namespace filesystem {


/// tcp/ip connection handler for queries sent by the client
class OutputChannel
{
    private:
        static const unsigned int sm_bufsize = 256;

        char    m_buf[sm_bufsize];  ///< socket buffer
        int     m_sockfd;           ///< socket file descriptor

        void cleanup();

    public:
        OutputChannel();

        // performs server handshake and authentication
        bool handshake( int sockfd );



};







} // namespace filesystem
} // namespace openbook




#endif // OUTPUTCHANNEL_H_

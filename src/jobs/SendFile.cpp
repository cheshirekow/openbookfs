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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/jobs/SendFile.cpp
 *
 *  @date   May 12, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <fcntl.h>

#include "SendFile.h"
#include "Backend.h"
#include "messages.h"



namespace   openbook {
namespace filesystem {
namespace       jobs {


void SendFile::go()
{
    path_t root     = m_backend->realRoot();
    path_t fullpath = root / m_path;

    struct stat fileStat;
    int result = stat( fullpath.c_str(), &fileStat );
    if( result < 0 )
        codedExcept(errno) << "Failed to stat file to send\n";

    int64_t size = fileStat.st_size;

    std::stringstream report;
    report << "SendFile: (" << m_path << ") starting up\n";
    std::cout << report.str();

    while( m_off < size )
    {
        // get the version of the file
        VersionVector v_curr;
        m_backend->db().getVersion( m_path, v_curr );

        // if the version has changed then abort the send
        if( v_curr != m_version )
        {
            ex()() << "SendFile: Local file " << m_path
                    << " changed during xfer";
        }

        // otherwise read in some bytes
        const int nBytes = 1024;
        char buf[nBytes];
        int fd = open(fullpath.c_str(), O_RDONLY );
        if( fd < 0 )
            codedExcept(errno) << "SendFile: Failed to open " << fullpath;

        // move the read head
        if( m_off != lseek(fd,m_off,SEEK_SET) )
        {
            close(fd);
            codedExcept(errno) << "SendFile: Failed to seek to offset "
                               << m_off
                               << " of file " << fullpath;
        }

        // read the data
        int bytesRead = read(fd,buf,nBytes);
        if( bytesRead < 0 )
        {
            close(fd);
            codedExcept(errno) << "SendFile: Failed to read from " << fullpath;
        }

        // close the file
        close(fd);

        // build the message
        messages::FileChunk* fileChunk = new messages::FileChunk();
        fileChunk->set_path(m_path.string());
        fileChunk->set_tx(m_tx);
        fileChunk->set_offset(m_off);
        fileChunk->set_data(buf,bytesRead);

        // increment the offset
        m_off += bytesRead;

        // send the message, if disconnected then quit
        if( !m_backend->sendMessage(m_peerId,fileChunk,PRIO_XFER) )
            break;
    }
}

} //< jobs
} //< filesystem
} //< openbook








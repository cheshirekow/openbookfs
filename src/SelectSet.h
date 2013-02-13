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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/SelectSet.h
 *
 *  @date   Feb 13, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_SELECTSET_H_
#define OPENBOOK_SELECTSET_H_

#include <vector>
#include <cstdlib>
#include <unistd.h>


namespace   openbook {
namespace filesystem {

/// stores a list of file descriptors to select() and manages reuse of the
/// fd_set
class SelectSet
{
    private:
        std::vector<int> m_fd;
        fd_set           m_set;
        timeval          m_timeout;
        int              m_maxfd;

    public:
        SelectSet(int numfd );

        /// access a file descriptor
        int& operator[]( unsigned int i_fd );

        /// returns true if the file descriptor with index i_fd is
        /// ready
        bool operator()( unsigned int i_fd );

        /// set the timeout
        void setTimeout( unsigned int sec, unsigned long int usec=0 );

        /// determines the max fd
        void init();

        /// calls select() and blocks until a file descriptor is
        /// ready
        int wait();

};



} // namespace filesystem
} // namespace openbook






#endif // SELECTSET_H_

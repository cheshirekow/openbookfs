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
#include <bitset>
#include <unistd.h>


namespace   openbook {
namespace filesystem {

/// an fd_set with array accessors
class FdSet
{
    public:
        /// reference to an element in a bitset
        class Ref
        {
            private:
                fd_set* m_set;
                int     m_fd;

                friend class FdSet;
                Ref(fd_set* set, int fd );

            public:
                operator bool() const;          ///< convert to bool
                Ref& operator=( bool x );       ///< assign from bool
                Ref& flip();                    ///< flip the bit
                bool operator~() const;         ///< inverse value
        };

        /// reference to an element in a bitset
        class ConstRef
        {
            private:
                const fd_set* m_set;
                int           m_fd;

                friend class FdSet;
                ConstRef(const fd_set* set, int fd );

            public:
                operator bool() const;          ///< convert to bool
                bool operator~() const;         ///< inverse value
        };

    private:
        fd_set  m_fdset;

    public:
        /// implicit conversion
        operator fd_set*();

        /// implicit conversion
        operator const fd_set*() const;

        /// clears (zeros) the set
        void clear();

        /// array operator
        Ref operator[]( unsigned int fd );
        const ConstRef operator[]( unsigned int fd ) const;
};


/// stores a list of file descriptors to select() and manages reuse of the
/// fd_set
class SelectSet
{
    public:
        enum Which
        {
            READ    =0,
            WRITE   =1,
            EXCEPT  =2,
            NUM_WHICH
        };

        /// dangerous, but I believe this can be cast to an fd_set;
        typedef std::bitset<FD_SETSIZE>  set_t;

    private:
        std::vector<int>    m_fd;
        FdSet               m_set[NUM_WHICH];
        timeval             m_timeout;
        int                 m_maxfd;

    public:
        SelectSet( );

        /// access / assign a file descriptor
        int& operator[]( unsigned int i_fd  );

        /// returns true if the file descriptor with index i_fd is ready,
        /// i.e. the operation specified in @p which will not block
        bool operator()( unsigned int i_fd, Which which=READ );

        /// set the timeout of the select
        void setTimeout( unsigned int sec, unsigned long int usec=0 );

        /// determines the max fd
        void init();

        /// calls select() and blocks until a file descriptor is
        /// ready
        int wait(Which which=READ);

};



} // namespace filesystem
} // namespace openbook






#endif // SELECTSET_H_

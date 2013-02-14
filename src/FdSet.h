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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/FdSet.h
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FDSET_H_
#define OPENBOOK_FDSET_H_

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



} // namespace filesystem
} // namespace openbook













#endif // FDSET_H_

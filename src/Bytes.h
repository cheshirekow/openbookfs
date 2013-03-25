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
 *  @file   src/client_fs/bytes.h
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_BYTES_H_
#define OPENBOOK_BYTES_H_

#include <cstdarg>

namespace   openbook {
namespace filesystem {

/// exposes individual bytes with array access operator
template <typename T>
class Bytes
{
    private:
        T* m_ptr;

    public:
        Bytes( T* ptr=0 ):
            m_ptr(ptr)
        {}

        unsigned int operator[]( unsigned int i )
        {
            unsigned char* ptr = (unsigned char*)m_ptr;
            return ptr[i];
        }

        std::size_t size()
        {
            return sizeof(T);
        }
};


} // namespace filesystem
} // namespace openbook







#endif // BYTES_H_

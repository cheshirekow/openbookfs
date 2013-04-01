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
 *  @file   src/server/Pool.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_POOL_H_
#define OPENBOOK_POOL_H_

#include <vector>
#include <cpp-pthreads.h>

namespace   openbook {
namespace filesystem {

/// locked pool of resources
template <class Obj>
class Pool
{
    private:
        pthreads::Mutex     m_mutex;        ///< locks the pool
        std::vector<Obj*>   m_available;    ///< available objects

    public:
        /// initializes the free store
        Pool(unsigned int size)
        {
            m_mutex.init();
            pthreads::ScopedLock lock(m_mutex);
            m_available.reserve(size);
        }

        ~Pool()
        {
            m_mutex.destroy();
        }

        /// retrieve an available handler
        Obj* getAvailable()
        {
            Obj* obj = 0;

            // lock scope
            {
                pthreads::ScopedLock lock(m_mutex);
                if( m_available.size() > 0 )
                {
                    obj = m_available.back();
                    m_available.pop_back();
                }
            }

            return obj;
        }

        /// reinsert a handler which has just become available
        void reassign( Obj* obj)
        {
            pthreads::ScopedLock lock(m_mutex);
            m_available.push_back(obj);
        }

        int size()
        {
            int size = 0;

            // lock scope
            {
                pthreads::ScopedLock lock(m_mutex);
                size = m_available.size();
            }
            return size;
        }
};



} // namespace filesystem
} // namespace openbook








#endif // POOL_H_

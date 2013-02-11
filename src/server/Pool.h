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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Pool.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_POOL_H_
#define OPENBOOK_POOL_H_



namespace   openbook {
namespace filesystem {

/// locked pool of resources
template <class Obj>
class Pool
{
    private:
        unsigned int        m_size;         ///< number of items
        pthreads::Mutex     m_mutex;        ///< locks the pool
        Obj*                m_objs;         ///< pool of objects
        std::vector<Obj*>   m_available;    ///< available objects

    public:
        /// initializes the free store
        Pool(unsigned int size):
            m_size(size)
        {
            m_mutex.init();
            pthreads::ScopedLock lock(m_mutex);

            // allocate storage
            m_objs = new Obj[size];

            m_available.reserve(size);
            for(int i=0; i < size; i++)
            {
                // initialize objects with reference to this pool
                m_objs[i].init(this);

                // but it in the free store
                m_available.push_back(m_objs + i);
            }
        }

        /// retrieve an available handler
        Obj* getAvailable()
        {
            Obj* obj = 0;

            // lock scope
            {
                pthreads::ScopedLock lock(m_mutex);
                if( m_available.size() == 0 )
                    return 0;

                obj = m_available.back();
                m_available.pop_back();
            }

            return obj;
        }

        /// reinsert a handler which has just become available
        void reassign( Obj* obj)
        {
            pthreads::ScopedLock lock(m_mutex);
            m_available.push_back(obj);
        }
};



} // namespace filesystem
} // namespace openbook








#endif // POOL_H_
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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/ReferenceCounted.h
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_REFERENCECOUNTED_H_
#define OPENBOOK_FS_REFERENCECOUNTED_H_

namespace   openbook {
namespace filesystem {

class ReferenceCounted
{
    protected:
        int m_refCount;

    public:
        ReferenceCounted():
            m_refCount(0)
        {}

        void reference()
        {
            m_refCount++;
        }

        bool dereference()
        {
            return --m_refCount <= 0;
        }
};


template <typename T>
struct RefPtr
{
    private:
        T*  m_ptr;

        void reference()
        {
            if(m_ptr)
                m_ptr->reference();
        }

        void dereference()
        {
            if(m_ptr)
            {
                if( m_ptr->dereference() )
                    delete m_ptr;
                m_ptr = 0;
            }
        }

    public:
        RefPtr( T* ptr = 0 ):
            m_ptr(ptr)
        {
            reference();
        }

        ~RefPtr()
        {
            dereference();
        }

        RefPtr( const RefPtr<T>& other ):
            m_ptr( other.m_ptr )
        {
            reference();
        }

        RefPtr<T>& operator=( const RefPtr<T>& other )
        {
            dereference();
            m_ptr = other.m_ptr;
            reference();
            return *this;
        }

        bool operator==( const RefPtr<T>& other ) const
        {
            return m_ptr == other.m_ptr;
        }

        bool operator<( const RefPtr<T>& other ) const
        {
            return m_ptr < other.m_ptr;
        }

        bool operator<=( const RefPtr<T>& other ) const
        {
            return m_ptr <= other.m_ptr;
        }

        bool operator>=( const RefPtr<T>& other ) const
        {
            return m_ptr >= other.m_ptr;
        }

        bool operator>( const RefPtr<T>& other ) const
        {
            return m_ptr > other.m_ptr;
        }

        T& operator*()
            { return *m_ptr; }

        const T& operator*() const
            {return *m_ptr;}

        T* operator->()
            { return m_ptr; }

        const T* operator->() const
            { return m_ptr; }

        operator bool()
        {
            return m_ptr;
        }

        T*  subvert()
            { return m_ptr; }

        const T* subvert() const
            { return m_ptr; }

        void clear()
        {
            dereference();
        }
};

} //< filesystem
} //< openbook















#endif // REFERENCECOUNTED_H_

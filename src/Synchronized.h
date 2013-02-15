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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/Synchronized.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_SYNCHRONIZED_H_
#define OPENBOOK_SYNCHRONIZED_H_




namespace   openbook {
namespace filesystem {


template <class Base> class Synchronized;

/// acts like a pointer to an object but the object is locked during it's
/// life time
template <class Base>
class LockedPtr
{
    friend class Synchronized<Base>;

    private:
        Synchronized<Base>&     m_synced;

        /// constructor is private, conly constructed by a Synchronized
        /// object, locks the synced object
        explicit LockedPtr( Synchronized<Base>& synced );

        /// not copyable
        LockedPtr( const LockedPtr& );

        /// not assignable
        LockedPtr<Base>& operator=( const LockedPtr& );

    public:
        /// release lock on synced object
        ~LockedPtr();

        /// expose underlying pointer
        Base* operator->();

        /// exposes underlying pointer
        const Base* operator->() const;
};





/// acts like another object but adds mutex locking
template <class Base>
class Synchronized
{
    friend class LockedPtr<Base>;

    private:
        pthreads::Mutex m_mutex;
        Base            m_base;

    public:
        /// when we access the object through pointer indirection we return
        /// an intermediate object which locks the mutex preventing others
        /// from acessing the object
        LockedPtr<Base> operator->()
        {
            return LockedPtr<Base>(*this);
        }

        /// return the base pointer, subverting protections
        Base* subvert()
        {
            return &m_base;
        }
};


template <class Base>
LockedPtr<Base>::LockedPtr( Synchronized<Base>& synced ):
    m_synced(synced)
{
    m_synced.m_mutex.lock();
}

template <class Base>
LockedPtr<Base>::~LockedPtr( )
{
    m_synced.m_mutex.unlock();
}

template <class Base>
Base* LockedPtr<Base>::operator->()
{
    return &(m_synced.m_base);
}

template <class Base>
const Base* LockedPtr<Base>::operator->() const
{
    return &(m_synced.m_base);
}







} // namespace filesystem
} // namespace openbook



#endif // SYNCHRONIZED_H_

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
 *  @file   src/Synchronized.h
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
template <class Base> class LockedPtr;
template <class Base> class ConstLockedPtr;


/// delegate returned from a synchronized object and passed to the LockedPtr
/// constructor
template <class Base>
class LockedPtrDelegate
{
    friend class LockedPtr<Base>;
    friend class ConstLockedPtr<Base>;
    friend class Synchronized<Base>;

    Synchronized<Base>* m_synced;

    Synchronized<Base>* getPtr() const { return m_synced; }
    LockedPtrDelegate( Synchronized<Base>* synced ):
        m_synced(synced)
    {}
};

/// acts like a pointer to an object but the object is locked during it's
/// life time
template <class Base>
class LockedPtr
{
    friend class Synchronized<Base>;

    private:
        Synchronized<Base>*     m_synced;

        /// not copyable
        LockedPtr( const LockedPtr& );

        /// not copyable
        LockedPtr<Base>& operator=( const LockedPtr& );

        /// called by constructor and assignment operator
        void assign( Synchronized<Base>* synced );

    public:
        /// constructor locks the object
        LockedPtr( Synchronized<Base>* synced=0 );

        /// assignment of a pointer releases lock on current pointer and
        /// aquires lock on new pointer
        LockedPtr<Base>& operator=( Synchronized<Base>* synced );

        /// constructor locks the object
        LockedPtr( LockedPtrDelegate<Base> delegate );

        /// assignment of a pointer releases lock on current pointer and
        /// aquires lock on new pointer
        LockedPtr<Base>& operator=( LockedPtrDelegate<Base> delegate );

        /// release lock on synced object
        ~LockedPtr();

        /// expose underlying pointer
        Base* operator->();

        /// exposes underlying pointer
        const Base* operator->() const;

        /// expose underlying object
        Base& operator*();

        /// exposes underlying object
        const Base& operator*() const;

        /// act as a boolean
        operator bool() const;
};

/// acts like a pointer to an object but the object is locked during it's
/// life time
template <class Base>
class ConstLockedPtr
{
    friend class Synchronized<Base>;

    private:
        Synchronized<Base>* m_synced;

        /// not copyable
        ConstLockedPtr( const ConstLockedPtr& );

        /// not copyable
        ConstLockedPtr<Base>& operator=( const ConstLockedPtr& );

        /// called by constructor and assignment operator
        void assign( Synchronized<Base>* synced );

    public:
        /// constructor locks the object
        ConstLockedPtr( Synchronized<Base>* synced );

        /// assignment of a pointer releases lock on current pointer and
        /// aquires lock on new pointer
        ConstLockedPtr<Base>& operator=( Synchronized<Base>* synced );

        /// constructor locks the object
        ConstLockedPtr( LockedPtrDelegate<Base> delegate );

        /// assignment of a pointer releases lock on current pointer and
        /// aquires lock on new pointer
        ConstLockedPtr<Base>& operator=( LockedPtrDelegate<Base> delegate );

        /// release lock on synced object
        ~ConstLockedPtr();

        /// exposes underlying pointer
        const Base* operator->() const;

        /// exposes underlying object
        const Base& operator*() const;

        /// act as a boolean
        operator bool() const;
};





/// acts like another object but adds mutex locking
template <class Base>
class Synchronized
{
    friend class LockedPtr<Base>;
    friend class ConstLockedPtr<Base>;

    private:
        pthreads::Mutex     m_mutex;  ///< locked if reference exists
        pthreads::Condition m_cond;   ///< signaled when reference destroyed
        Base                m_base;

    public:
        Synchronized()
        {
            m_mutex.init();
            m_cond.init();
        }

        ~Synchronized()
        {
            m_mutex.destroy();
            m_cond.destroy();
        }

        /// when we access the object through pointer indirection we return
        /// an intermediate object which locks the mutex preventing others
        /// from acessing the object
        LockedPtr<Base> operator->()
        {
            return LockedPtr<Base>(this);
        }

        ConstLockedPtr<Base> operator->() const
        {
            return ConstLockedPtr<Base>(this);
        }

        LockedPtr<Base> lockFor()
        {
            return LockedPtr<Base>(this);
        }

        ConstLockedPtr<Base> constLockFor()
        {
            return ConstLockedPtr<Base>(this);
        }

        LockedPtrDelegate<Base> lockedPtr()
        {
            return LockedPtrDelegate<Base>(this);
        }

        /// explicitly lock
        int lock(){ return m_mutex.lock(); }

        /// explicitly unlock
        int unlock(){ return m_mutex.unlock(); }

        /// wait for a change to be signaled, note the mutex must be owned
        /// by the calling thread, either by a lock() call or by explicitly
        /// locking the mutex returned by mutex()
        void wait(){ m_cond.wait(m_mutex); }

        /// explicitly signal the condition
        void signal(){ m_cond.signal(); }

        /// for using a scoped lock
        pthreads::Mutex& mutex(){ return m_mutex; }

        /// return the base pointer, subverting protections
        Base* subvert()
        {
            return &m_base;
        }

        /// return the base pointer, subverting protections
        const Base* subvert() const
        {
            return &m_base;
        }
};








template <class Base>
void LockedPtr<Base>::assign( Synchronized<Base>* synced )
{
    if(m_synced)
        m_synced->unlock();
    m_synced = synced;
    m_synced->lock();
}


template <class Base>
LockedPtr<Base>::LockedPtr( Synchronized<Base>* synced ):
    m_synced(0)
{
    assign(synced);
}

template <class Base>
LockedPtr<Base>& LockedPtr<Base>::operator=( Synchronized<Base>* synced )
{
    assign(synced);
    return *this;
}

template <class Base>
LockedPtr<Base>::LockedPtr( LockedPtrDelegate<Base> delegate ):
    m_synced(0)
{
    assign( delegate.getPtr() );
}

template <class Base>
LockedPtr<Base>& LockedPtr<Base>::operator=( LockedPtrDelegate<Base> delegate )
{
    assign( delegate.getPtr() );
    return *this;
}



template <class Base>
LockedPtr<Base>::~LockedPtr( )
{
    if(m_synced)
    {
        m_synced->signal();
        m_synced->unlock();
    }
}

template <class Base>
Base* LockedPtr<Base>::operator->()
{
    return m_synced->subvert();
}

template <class Base>
const Base* LockedPtr<Base>::operator->() const
{
    return m_synced->subvert();
}


template <class Base>
Base& LockedPtr<Base>::operator*()
{
    return *(m_synced->subvert());
}

template <class Base>
const Base& LockedPtr<Base>::operator*() const
{
    return *(m_synced->subvert());
}

template <class Base>
LockedPtr<Base>::operator bool() const
{
    return m_synced->subvert();
}




template <class Base>
void ConstLockedPtr<Base>::assign( Synchronized<Base>* synced )
{
    if(m_synced)
        m_synced->unlock();
    m_synced = synced;
    m_synced->lock();
}


template <class Base>
ConstLockedPtr<Base>::ConstLockedPtr( Synchronized<Base>* synced ):
    m_synced(0)
{
    assign(synced);
}

template <class Base>
ConstLockedPtr<Base>& ConstLockedPtr<Base>::operator=( Synchronized<Base>* synced )
{
    assign(synced);
    return *this;
}

template <class Base>
ConstLockedPtr<Base>::ConstLockedPtr(LockedPtrDelegate<Base> delegate ):
    m_synced(0)
{
    assign( delegate.getPtr() );
}

template <class Base>
ConstLockedPtr<Base>& ConstLockedPtr<Base>::operator=( LockedPtrDelegate<Base> delegate )
{
    assign( delegate.getPtr() );
    return *this;
}

template <class Base>
ConstLockedPtr<Base>::~ConstLockedPtr( )
{
    if(m_synced)
    {
        m_synced->signal();
        m_synced->unlock();
    }
}

template <class Base>
const Base* ConstLockedPtr<Base>::operator->() const
{
    return m_synced->subvert();
}

template <class Base>
const Base& ConstLockedPtr<Base>::operator*() const
{
    return *(m_synced->subvert());
}

template <class Base>
ConstLockedPtr<Base>::operator bool() const
{
    return m_synced->subvert();
}







} // namespace filesystem
} // namespace openbook



#endif // SYNCHRONIZED_H_

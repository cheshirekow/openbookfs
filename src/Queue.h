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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/Queue.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_QUEUE_H_
#define OPENBOOK_QUEUE_H_

#include "ExceptionStream.h"
#include <cpp-pthreads.h>

namespace   openbook {
namespace filesystem {


/// a node for the queue, doubly linked list
/**
 *  @note   simply stores a T and a pointer to the next and previous node
 */
template <typename T>
struct Node
{
    typedef Node<T> This_t;

    T       data;
    This_t* next;
    This_t* prev;

    /// constructor, initializes with blank data
    Node( ):
        next(0),
        prev(0)
    {}

    /// insert into a queue after the specified node
    This_t* insertAfter(This_t* parent)
    {
        prev  = parent;
        next  = parent->next;

        if(prev)
            prev->next = this;
        if(next)
            next->prev = this;

        return this;
    }


    /// insert into a queue before the specified node
    This_t* insertBefore(This_t* child)
    {
        prev  = child->prev;
        next  = child;

        if(prev)
            prev->next = this;
        if(next)
            next->prev = this;

        return this;
    }

    /// remove from a queue
    void extract()
    {
        if(prev)
            prev->next = next;
        if(next)
            next->prev = prev;

        prev = 0;
        next = 0;
    }
};


/// a FIFO queue with mutually exclusive access and provides condition
/// signalling for when something has been added or removed
/**
 *  Queues have a static maximum size. Nodes are allocated only once during
 *  Construction
 *  @note   items in the queue are stored in a singly linked list
 */
template <typename T>
class Queue
{
    public:
        typedef Node<T> Node_t;

    private:
        pthreads::Mutex         m_mutex;    ///< locks the queue
        pthreads::Condition     m_new;      ///< signals a new item added to
                                            ///  the queue

        Node_t*  m_memblock;    ///< block to deallocate when destroyed
        Node_t*  m_firstFree;   ///< first unusued node
        Node_t*  m_lastFree;    ///< last unused node

        Node_t*  m_first;    ///< first node in the queue
        Node_t*  m_last;     ///< last node in the queue

        /// remove a node from the free chain and return it
        Node_t* alloc()
        {
            Node_t* node = m_firstFree;
            if( node )
            {
                // advance the front of the queue
                m_firstFree = node->next;

                // remove this node from the chain
                node->extract();

                // update the tail of the queue if necessary
                if( node == m_lastFree )
                    m_lastFree = 0;
            }
            return node;
        }

        /// insert a node into the free chain (assume it has already been
        /// removed from the queue chain
        void free( Node_t* node )
        {
            /// if the free chain is not empty, then just insert this
            /// one at the back
            if(m_lastFree)
                node->insertAfter(m_lastFree);

            /// if it is empty then this node is both the front and the
            /// back
            else
                m_firstFree = node;

            /// in either case it is the new back of the free chain
            m_lastFree = node;
        }



    public:
        /// construct a queue with a maximum storage size of @p size elements
        Queue( unsigned int size=100 )
        {
            m_mutex.init();
            m_new.init();

            // allocate storage
            m_memblock = new Node_t[size];

            // put all the nodes in the free queue
            m_firstFree = m_lastFree = m_memblock;

            for( int i=1; i < size; i++ )
            {
                m_memblock[i].insertAfter(m_lastFree);
                m_lastFree = m_memblock + i;
            }

            // initialize the queue to empty
            m_first = m_last = 0;
        }

        /// free storage nodes
        ~Queue()
        {
            m_mutex.destroy();
            m_new.destroy();
            delete [] m_memblock;
        }

        /// insert an element into the queue
        void insert( T& data )
        {
            // lock the queue during this call
            pthreads::ScopedLock lock(m_mutex);

            // first get a free node structure
            Node_t* node = alloc();

            if(!node)
                ex()() << "Attempt to insert element into full queue";

            // set the data
            node->data = data;

            // now insert it into the element queue
            if(m_last)
                node->insertAfter(m_last);
            else
                m_first = node;
            m_last = node;

            // if there is any thread waiting for new data then signal that
            // thread, note that we still own the mutex until after this
            // call finishes so there is no contention for the queue
            m_new.signal();
        }

        /// extract one element from the queue
        void extract( T& data )
        {
            // lock the queue and try to get something out of it
            pthreads::ScopedLock lock(m_mutex);

            // if there is nothing available, then wait for something
            // to become available
            if(!m_first)
            {
                // when we start waiting we release the mutex, but when this
                // call blocks it will not return until someone signals a new
                // item is available. When it does return we own the mutex
                // again so we can safely assume that there is an item
                // available and that we own the mutex so no one else will
                // be able to take it before we do
                m_new.wait(m_mutex);
            }

            if(!m_first)
                ex()() << "New item was signalled but no item in the queue";

            // remove the first element from the queue
            Node_t* node = m_first;
            m_first = node->next;
            node->extract();
            if( node == m_last )
                m_last = 0;

            // retrieve the data
            data = node->data;

            // return the node to the free chain
            free(node);
        }



};


} // namespace filesystem
} // namespace openbook





#endif // QUEUE_H_

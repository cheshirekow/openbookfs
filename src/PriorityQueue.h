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
 *  @file   src/PriorityQueue.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_PRIORITYQUEUE_H_
#define OPENBOOK_PRIORITYQUEUE_H_

#include "ExceptionStream.h"
#include <cpp-pthreads.h>

namespace   openbook {
namespace filesystem {



namespace     pqueue {


/// simple doubly linked list node, CRTP
template <typename Derived>
class Node
{
    public:
        typedef Node<Derived> Node_t;

    private:
        Derived* m_prev;
        Derived* m_next;

    public:
        /// constructor, initializes with null pointers
        Node():
            m_prev(0),
            m_next(0)
        {}

        Derived* prev(){ return m_prev; }
        Derived* next(){ return m_next; }

        /// insert into a queue after the specified node
        void insertAfter(Derived* parent)
        {
            m_prev  = parent;
            m_next  = static_cast<Node_t*>(parent)->m_next;

            if(m_prev)
                static_cast<Node_t*>(m_prev)->m_next =
                        static_cast<Derived*>(this);
            if(m_next)
                static_cast<Node_t*>(m_next)->m_prev =
                        static_cast<Derived*>(this);
        }


        /// insert into a queue before the specified node
        void insertBefore(Derived *child)
        {
            m_prev  = static_cast<Node_t*>(child)->m_prev;
            m_next  = child;

            if(m_prev)
                static_cast<Node_t*>(m_prev)->m_next =
                        static_cast<Derived*>(this);
            if(m_next)
                static_cast<Node_t*>(m_next)->m_prev =
                        static_cast<Derived*>(this);
        }

        /// remove from a chain
        void remove()
        {
            if(m_prev)
                static_cast<Node_t*>(m_prev)->m_next = m_next;
            if(m_next)
                static_cast<Node_t*>(m_next)->m_prev = m_prev;

            m_prev = 0;
            m_next = 0;
        }
};


/// doubly linked list
template <typename Derived>
class Queue
{
    public:
        typedef Node<Derived> Node_t;

    private:
        Derived* m_first;
        Derived* m_last;

    public:
        Queue():
            m_first(0),
            m_last(0)
        {}

        Derived* peek_back()
        {
            return m_last;
        }

        void push_back( Derived* node )
        {
            // now insert it into the element queue
            if(m_last)
                node->insertAfter(m_last);
            else
                m_first = node;
            m_last = node;
        }

        Derived* pop_back()
        {
            Derived* node = m_last;
            if(node)
                m_last = node->prev();;
            node->remove();
            if( node == m_first )
                m_first = 0;

            return node;
        }

        Derived* peek_front()
        {
            return m_first;
        }

        void push_front( Derived* node )
        {
            if(m_first)
                node->insertBefore(m_first);
            else
                m_last = node;
            m_first = node;
        }

        Derived* pop_front()
        {
            // remove the first element from the queue
            Derived* node = m_first;
            if(node)
                m_first = node->next();
            node->remove();
            if( node == m_last )
                m_last = 0;

            return node;
        }
};


/// level 2 node for the queue, doubly linked list, stores some object
template <typename Storage>
struct DataNode:
    public Node< DataNode<Storage> >
{
    Storage data;
};








} // namespace pqueue


/// a priority FIFO with mutually exclusive access and condition
/// signalling for when something has been added or removed
/**
 *  PriorityQueues have a static maximum size. Nodes are allocated only once during
 *  Construction
 *  @note   items in the queue are stored in a singly linked list
 */
template <typename T>
class PriorityQueue
{
    public:
        typedef pqueue::DataNode<T>         DataNode_t;
        typedef pqueue::Queue<DataNode_t>   DataQueue_t;
        typedef std::vector<DataQueue_t>    QueueStore_t;
        typedef std::vector<DataQueue_t>    FreeStore_t;
        typedef std::vector<DataNode_t>     DataStore_t;
        typedef std::set<int>               PrioQueue_t;
        typedef std::vector<pthreads::Condition>    CondVec_t;


    private:
        pthreads::Mutex         m_mutex;    ///< locks the queue
        pthreads::Condition     m_notEmpty; ///< signals a new item added to
                                            ///  the queue
        CondVec_t               m_notFull;  ///< signals that the queue has
                                            ///  availabile capacity

        DataStore_t  m_dataStore;   ///< allocated data blocks
        QueueStore_t m_queueStore;  ///< stores unused priority blocks
        FreeStore_t  m_freeStore;   ///< stores unused data blocks
        PrioQueue_t  m_prioQueue;   ///< the queue

    public:
        /// construct a queue with a maximum storage size of @p size elements
        /**
         *  @param nprio     number of priority levels
         *  @param size     maximum storage size for each priority level
         */
        PriorityQueue( unsigned int nprio=4, unsigned int size=25 )
        {
            m_mutex.init();
            m_notEmpty.init();

            // allocate storage
            m_notFull.resize(nprio);
            m_queueStore.resize(nprio);
            m_freeStore.resize(nprio);
            m_dataStore.resize(nprio*size);

            // put all the nodes in the free queue
            for( int i=0; i < nprio; i++ )
            {
                m_notFull[i].init();
                for(int j=0; j < size; j++ )
                    m_freeStore[i].push_back( &m_dataStore[i*size + j] );
            }
        }

        /// free storage nodes
        ~PriorityQueue()
        {
            m_mutex.destroy();
            m_notEmpty.destroy();
            for( auto& cond : m_notFull )
                cond.destroy();
        }

        /// insert an element into the queue
        void insert( T data, int prio=0 )
        {
            if( prio > m_queueStore.size() )
                ex()() << "Attempt to insert into priority queue with "
                          "priority " << prio << " where max priority "
                          "is " << m_queueStore.size();

            // lock the queue during this call
            pthreads::ScopedLock lock(m_mutex);

            // get the queue at the specified priority
            DataQueue_t& queue = m_queueStore[prio];

            // get a node to store the data
            DataNode_t* dNode = m_freeStore[prio].pop_back();

            // if there is no free node available then we wait for capacity
            // to be restored
            while(!dNode)
            {
                std::cout << "PriorityQueue: No available storage for priority "
                          << prio << " waiting for a free'd block";

                // when we enter the wait we release the mutex, so someone
                // else may put something into the queue... when we return
                // from the wait we own the mutex again so we cannot be
                // pre-empted
                m_notFull[prio].wait(m_mutex);
                dNode = m_freeStore[prio].pop_back();

                if(dNode)
                    std::cout << "PriorityQueue: got a free block, inserting"
                                 " item into queue at priority " << prio
                               << "\n";
            }

            // set the data
            dNode->data = data;

            // now put it into the priority queue
            queue.push_back(dNode);

            // ensure that this priority is in the queue
            m_prioQueue.insert(prio);

            // if there is any thread waiting for new data then signal that
            // thread, note that we still own the mutex until after this
            // call finishes so there is no contention for the queue
            m_notEmpty.signal();
        }

        /// extract one element from the queue
        void extract( T& data )
        {
            typedef typename PrioQueue_t::iterator  iterator;

            // lock the queue and try to get something out of it
            pthreads::ScopedLock lock(m_mutex);

            // if there is nothing available, then wait for something
            // to become available
            while( m_prioQueue.size() < 1 )
            {
                std::cout << "PriorityQueue: nothing to read from prioirty "
                             "queue, waiting\n";

                // when we start waiting we release the mutex, but when this
                // call blocks it will not return until someone signals a new
                // item is available. When it does return we own the mutex
                // again so we can safely assume that there is an item
                // available and that we own the mutex so no one else will
                // be able to take it before we do
                m_notEmpty.wait(m_mutex);
            }

            // get the highest priority (lowest numbered) non-empty queue
            iterator      it    = m_prioQueue.begin();
            int           prio  = *it;
            DataQueue_t&  queue = m_queueStore[prio];
            DataNode_t*   dNode = queue.pop_front();

            // if the queue is now empty then also remove it from the list of
            // non empty queues
            if( !queue.peek_front() )
                m_prioQueue.erase(it);

            // retrieve the data
            data = dNode->data;

            // return the node to the free chain
            m_freeStore[prio].push_back(dNode);

            // if there are any threads waiting for capacity to insert
            // into the queue, then release them
            m_notFull[prio].signal();
        }

        /// returns true if the queue is empty
        bool empty()
        {
            pthreads::ScopedLock lock(m_mutex);
            return m_prioQueue.size() == 0;
        }
};


} // namespace filesystem
} // namespace openbook





#endif // PRIORITYQUEUE_H_

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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/LockFreeQueue.h
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief
 *  @remark taken from
 *          http://www.drdobbs.com/high-performance-computing/210604448
 */

#ifndef OPENBOOK__LOCKFREEQUEUE_HPP_
#define OPENBOOK__LOCKFREEQUEUE_HPP_


namespace   openbook {
namespace filesystem {


/// a thread safe queue for a single producer and a single consumer thread that
/// does not rely on locks so induces no mutex overhead
template <typename T>
class LockFreeQueue
{
    private:
        struct Node
        {
            Node( T val ) : value(val), next(0) { }
            T       value;
            Node*   next;
        };

        Node*           first;              // for producer only
        volatile Node*  divider;
        volatile Node*  last;               // shared

    public:
        LockFreeQueue()
        {
            divider = last = first = new Node( T() );  // add dummy separator
        }

        ~LockFreeQueue()
        {
            // free all nodes in the list
            while( first != 0 )
            {
                Node*   tmp = first;
                first       = tmp->next;
                delete tmp;
            }
        }

        /// called by producer thread only
        void produce( const T& t )
        {
            last->next = new Node(t);   // add the new item
            last = last->next;          // publish it

            // trim unused nodes
            while( first != divider )
            {
                Node* tmp = first;
                first = first->next;
                delete tmp;
            }
        }


        /// called only by the consumer
        bool consume( T& result )
        {
            // if queue is nonempty
            if( divider != last )
            {
                result = divider->next->value;  // C: copy it back
                divider = divider->next;        // D: publish that we took it
                return true;                    // and report success
            }
            return false;                       // else report empty
        }
};








} // namespace filesystem
} // namespace openbook






#endif /* LOCKFREEQUEUE_H_ */

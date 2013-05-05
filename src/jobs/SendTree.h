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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/jobs/SendTree.h
 *
 *  @date   May 5, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_SENDTREE_H_
#define OPENBOOK_FS_SENDTREE_H_


#include "LongJob.h"

namespace   openbook {
namespace filesystem {

class Backend;

} //< filesystem
} //< openbook



namespace   openbook {
namespace filesystem {
namespace       jobs {


/// navigates the entire directory structure and sends version information
/// for all subscribed files (including directories)
class SendTree:
    public LongJob
{
    private:
        Backend*    m_backend;  ///< the backend object
        int         m_peerId;   ///< the peer to send to

    public:
        SendTree(Backend* backend, int peerId ):
            m_backend(backend),
            m_peerId(peerId)
        {}

        virtual ~SendTree(){}

        /// navigates the entire file system and sends version information
        /// to the connected peer
        virtual void go();


};


} //< jobs
} //< filesystem
} //< openbook




#endif // SENDTREE_H_

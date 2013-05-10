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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/jobs/SendTree.cpp
 *
 *  @date   May 5, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <iostream>
#include <boost/filesystem.hpp>
#include "Backend.h"
#include "SendTree.h"
#include "ExceptionStream.h"
#include "MetaFile.h"

namespace   openbook {
namespace filesystem {
namespace       jobs {


//static boost::filesystem::path make_relative(
//        const boost::filesystem::path& base,
//        const boost::filesystem::path& query )
//{
//    namespace fs = boost::filesystem;
//    fs::path::const_iterator baseIter( base.begin() );
//    fs::path::const_iterator baseEnd( base.end() );
//    fs::path::const_iterator queryIter( query.begin() );
//    fs::path::const_iterator queryEnd( query.end() );
//
//    // iterate over common prefix
//    for( ; baseIter != baseEnd
//            && queryIter != queryEnd
//            && *baseIter == *queryIter;
//            ++baseIter, ++queryIter );
//
//    if( baseIter != baseEnd )
//        ex()() << base << " is not a prefix of " << query;
//
//    fs::path result;
//    for( ; queryIter != queryEnd; ++queryIter )
//        result /= *queryIter;
//
//    return result;
//}

void SendTree::go()
{
    namespace fs = boost::filesystem;
    namespace msg = messages;

    // send a peer map message
    msg::IdMap* idMap = new msg::IdMap();
    m_backend->buildPeerMap(idMap);
    m_backend->sendMessage(m_peerId,idMap,0);

    fs::path root = m_backend->realRoot();

    std::list<fs::path> queue;
    queue.push_back(fs::path("./"));

    // pointer to message to send
    msg::DirChunk* chunk = 0;

    while(queue.size() > 0)
    {
        // get the next directory from the queue
        fs::path dir = queue.front();
        queue.pop_front();

        // create a chunk unless we didn't use it at the last round
        if(!chunk)
            chunk = new msg::DirChunk();

        // get the metadata file
        MetaFile meta( root / dir );

        // get the contents
        meta.readdir( chunk );

        // if the directory has children then recruse on any subidrs
        std::cout << "SendTree::go() : built directory message: "
                  << "\n directory : " << dir
                  << "\n contents  : \n";
        for(int i=0 ; i < chunk->entries_size(); i++)
        {
            fs::path subdir = dir / chunk->entries(i).path();
            std::cout << "    " << subdir << "\n";
            if( fs::is_directory(subdir) )
                queue.push_back(subdir);
        }

        // if the directory has contents, then send the message and add
        // children to the queue
        if( chunk->entries_size() > 0 )
        {
            int ok = m_backend->sendMessage(m_peerId,chunk,1);
            chunk = 0;

            // if the peer isn't connected dont continue
            if(!ok)
                break;
        }
    }

    if(chunk)
        delete chunk;

}


} //< jobs
} //< filesystem
} //< openbook




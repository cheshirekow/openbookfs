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
#include "VersionVector.h"

namespace   openbook {
namespace filesystem {
namespace       jobs {


static boost::filesystem::path make_relative(
        const boost::filesystem::path& base,
        const boost::filesystem::path& query )
{
    namespace fs = boost::filesystem;
    fs::path::const_iterator baseIter( base.begin() );
    fs::path::const_iterator queryIter( query.begin() );

    std::stringstream report;
    report << "make_relative:\n";

    // iterate over common prefix
    while( baseIter != base.end()
            && queryIter != query.end()
            && *baseIter == *queryIter )
    {
        report << "   " << baseIter->string()
                << " == " << queryIter->string() << "\n";
        ++baseIter;
        ++queryIter;
    }

//    std::cout << report.str();

//    if( baseIter != end )
//        ex()() << base << " is not a prefix of " << query
//               << " specifically " << *baseIter
//               << " != " << *queryIter;

    fs::path result;
    for( ; queryIter != query.end(); ++queryIter )
        result /= *queryIter;

    return result;
}

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
    queue.push_back(fs::path("."));

    std::list<fs::path> files;

    while(queue.size() > 0)
    {
        files.clear();

        // get the next directory from the queue
        fs::path dir = queue.front();
        queue.pop_front();

        // create a chunk unless we didn't use it at the last round
        // pointer to message to send
        msg::DirChunk* chunk = new msg::DirChunk();

        // get the metadata file
        MetaFile meta( root / dir );

        // get the contents
        meta.readdir( chunk );

        // if the directory has children then recurse on any subidrs
        std::cout << "SendTree::go() : built directory message: "
                  << "\n directory : " << dir
                  << "\n contents  : \n";
        for(int i=0 ; i < chunk->entries_size(); i++)
        {
            fs::path subpath = chunk->entries(i).path();
            std::cout << "    " << (dir / subpath) << "\n";
            if( fs::is_directory(root/dir/subpath) )
                queue.push_back(dir/subpath);
        }

        if( !m_backend->sendMessage(m_peerId,chunk,PRIO_SYNC) )
            break;

        // iterate over checked out files (i.e. files with actual
        // filesystem entires)
        fs::directory_iterator  it(root/dir);
        fs::directory_iterator  end;
        for( ; it != end; ++it )
        {
            fs::path subpath = make_relative(root/dir,*it);

            if( subpath.string() == "obfs.sqlite" )
                continue;

            fs::path fullpath = root/dir/subpath;
            fs::file_status status = fs::status(fullpath);

            msg::NodeType ntype = msg::DIRECTORY;
            switch( status.type() )
            {
                case fs::regular_file:
                    ntype = msg::REGULAR;
                    break;

                case fs::symlink_file:
                    ntype = msg::SIMLINK;
                    break;

                case fs::fifo_file:
                    ntype = msg::PIPE;
                    break;

                case fs::socket_file:
                    ntype = msg::SOCKET;
                    break;

                default:
                    break;
            }

            if( ntype == msg::DIRECTORY )
            {
                std::stringstream report;
                report << "SendTree: " << fullpath
                       << " is not sendable, etc\n";
                std::cout << report.str();
                continue;
            }

            struct stat statBuf;
            if( stat( fullpath.c_str(), &statBuf ) )
            {
                std::stringstream report;
                report << "SendTree: failed to stat "
                        << fullpath << " errno: " << errno
                        << ", " << strerror(errno) << "\n";
                std::cout << report.str();
                continue;
            }

            msg::NodeInfo* nodeInfo = new msg::NodeInfo();
            nodeInfo->set_parent(dir.string());
            nodeInfo->set_path(subpath.string());
            nodeInfo->set_mode(statBuf.st_mode);
            nodeInfo->set_size(statBuf.st_size);
            nodeInfo->set_ctime(statBuf.st_ctim.tv_sec);
            nodeInfo->set_mtime(statBuf.st_mtim.tv_sec);
            nodeInfo->set_type( ntype );

            VersionVector version;
            meta.getVersion(subpath.string(),version);

            for( auto& pair : version )
            {
                msg::VersionEntry* entry = nodeInfo->add_version();
                entry->set_client( pair.first );
                entry->set_version( pair.second );
            }

            // send the message
            if( !m_backend->sendMessage(m_peerId,nodeInfo,PRIO_SYNC) )
                break;
        }

    }
}


} //< jobs
} //< filesystem
} //< openbook




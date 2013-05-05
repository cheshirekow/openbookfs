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

namespace   openbook {
namespace filesystem {
namespace       jobs {


static boost::filesystem::path make_relative(
        const boost::filesystem::path& base,
        const boost::filesystem::path& query )
{
    namespace fs = boost::filesystem;
    fs::path::const_iterator baseIter( base.begin() );
    fs::path::const_iterator baseEnd( base.end() );
    fs::path::const_iterator queryIter( query.begin() );
    fs::path::const_iterator queryEnd( query.end() );

    // iterate over common prefix
    for( ; baseIter != baseEnd
            && queryIter != queryEnd
            && *baseIter != *queryIter;
            ++baseIter, ++queryIter );

    if( baseIter != baseEnd )
        ex()() << base << " is not a prefix of " << query;

    fs::path result;
    for( ; queryIter != queryEnd; ++queryIter )
        result /= *queryIter;

    return result;
}

void SendTree::go()
{
    namespace fs = boost::filesystem;
    fs::path root = m_backend->realRoot();

    for( fs::recursive_directory_iterator end, dir(root);
            dir != end; ++dir )
    {
        fs::path subpath = make_relative( root, dir->path() );
        std::cout << dir->path() << "\n";
    }
}


} //< jobs
} //< filesystem
} //< openbook




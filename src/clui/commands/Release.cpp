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
 *  @file   src/clui/commands/Release.cpp
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <sys/xattr.h>
#include <cerrno>

#include "global.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "Release.h"



namespace   openbook {
namespace filesystem {
namespace       clui {

const std::string Release::COMMAND       = "release";
const std::string Release::DESCRIPTION   = "unsubscribe from a file";

Release::Release( TCLAP::CmdLine& cmd ):
    Options(cmd),
    file(
        "file",
        "file to release",
        true,
        "",
        "file",
        cmd
        )
{}

void Release::go()
{
    std::stringstream path;
    path << "./" << file.getValue();

    int result = getxattr( path.str().c_str(), "obfs:release", 0, 0 );
    if( result < 0 )
        codedExcept(errno)() << "Failed to checkout " << file.getValue();
}





} //< namespace clui
} //< namespace filesystem
} //< namespace openbook




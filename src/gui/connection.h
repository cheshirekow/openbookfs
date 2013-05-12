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
 *  @file   src/gui/connection.h
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_GUI_CONNECTION_H_
#define OPENBOOK_FS_GUI_CONNECTION_H_

#include <cerrno>
#include <dirent.h>
#include <netdb.h>
#include <sys/time.h>
#include <iostream>

#include "Options.h"
#include "FileDescriptor.h"
#include "Marshall.h"
#include "ReferenceCounted.h"


namespace   openbook {
namespace filesystem {
namespace       gui {


typedef RefPtr<FileDescriptor>  FdPtr_t;

/// create a connection to the desired server
FdPtr_t connectToClient( Options& opts );

/// perform handshake to notify client that we are a ui
void handshake( Marshall& marshall );


} //< namespace gui
} //< namespace filesystem
} //< namespace openbook



#endif // CONNECTION_H_

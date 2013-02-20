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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/ClientMap.h
 *
 *  @date   Feb 20, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_CLIENTMAP_H_
#define OPENBOOK_CLIENTMAP_H_

#include <map>
#include <stdint.h>

#include "Synchronized.h"



namespace   openbook {
namespace filesystem {
namespace     server {

/// maps client ids to client handlers, if the client is connected
typedef Synchronized< std::map<uint64_t,ClientHandler*> >   ClientMap;





} // namespace server
} // namespace filesystem
} // namespace openbook











#endif // CLIENTMAP_H_

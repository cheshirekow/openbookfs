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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/global.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_GLOBAL_H_
#define OPENBOOK_GLOBAL_H_


#include <cpp-pthreads.h>
#include "NotifyPipe.h"


namespace   openbook {
namespace filesystem {
 namespace    server {

extern
pthreads::Key g_handlerKey;  ///< associates the ClientHandler* pointer with
                             ///  each handler thread
extern
NotifyPipe* g_termNote;      ///< pipe used to break out of select statements


} // namespace server 
} // namespace filesystem
} // namespace openbook


#endif // GLOBAL_H_

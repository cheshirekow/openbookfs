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
 *  @file   msg_gen/MessageStr.h
 *
 *  @date   Apr 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  Generated by message_map.pl
 */

#ifndef OPENBOOK_FS_MSG_GEN_MESSAGESTR_H
#define OPENBOOK_FS_MSG_GEN_MESSAGESTR_H

#include "msg_gen/MessageId.h"

namespace   openbook {
namespace filesystem {
    
const char* messageIdToString( MessageId id );

    
} //< namespace filesystem 
} //< namespace openbook

#endif //< OPENBOOK_FS_MSG_GEN_MESSAGESTR_H
        
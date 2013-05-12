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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/gui/commands/StartSync.h
 *
 *  @date   May 5, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_GUI_STARTSYNC_H_
#define OPENBOOK_ST_GUI_STARTSYNC_H_


#include <tclap/CmdLine.h>

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

class StartSync:
    public Options
{
    TCLAP::UnlabeledValueArg<int> peerId;    ///< peer to sync

    public:
        static const std::string COMMAND;
        static const std::string DESCRIPTION;

        StartSync();
        void go();
};



} //< namespace gui
} //< namespace filesystem
} //< namespace openbook

#endif // STARTSYNC_H_

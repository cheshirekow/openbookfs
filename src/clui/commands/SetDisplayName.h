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
 *  @file   src/clui/SetDisplayName
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_CLUI_DISPLAYNAMEOPTIONS_H_
#define OPENBOOK_FS_CLUI_DISPLAYNAMEOPTIONS_H_

#include "Options.h"

namespace   openbook {
namespace filesystem {
namespace       clui {

class SetDisplayName:
    public Options
{
    TCLAP::UnlabeledValueArg<std::string> displayName;    ///< display name

    public:
        static const std::string COMMAND;
        SetDisplayName( TCLAP::CmdLine& cmd );
        void go();
};


} //< namespace clui
} //< namespace filesystem
} //< namespace openbook





#endif // DISPLAYNAMEOPTIONS_H_

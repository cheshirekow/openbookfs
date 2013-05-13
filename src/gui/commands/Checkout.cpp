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
 *  @file   src/clui/commands/Checkout.cpp
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
#include "Checkout.h"
#include <QDebug>


namespace   openbook {
namespace filesystem {
namespace       gui {

const std::string Checkout::COMMAND       = "checkout";
const std::string Checkout::DESCRIPTION   = "subscribe to a file";

Checkout::Checkout(QString port):
    Options(port)

{}

void Checkout::go(QString filename)
{

    int result = getxattr( filename.toUtf8().constData(), "obfs:checkout", 0, 0 );
    if( result < 0 )
        qDebug() << "Failed to checkout " << filename;
}





} //< namespace clui
} //< namespace filesystem
} //< namespace openbook




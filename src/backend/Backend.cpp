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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/Backend.cpp
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "global.h"
#include "Backend.h"

namespace   openbook {
namespace filesystem {

Backend::Backend()
{

}

Backend::~Backend()
{

}

int Backend::run()
{
    // for notifiying termination
    g_termNote = &m_termNote;

    // start the socket listeners
    for(int i=0; i < NUM_LISTENERS; i++)
        m_listenThreads[i].launch( SocketListener::start, m_listeners+i );

    sleep(2);
    for(int i=0; i < NUM_LISTENERS; i++)
        m_listeners[i].setInterface( "localhost", 3030+i);

    sleep(2);
    g_termNote->notify();

    for(int i=0; i < NUM_LISTENERS; i++)
        m_listenThreads[i].join();

    return 0;
}

}
}

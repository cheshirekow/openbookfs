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
 *  @file   src/client_fs/main.cpp
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  The main() entry point for the client program
 */

#include <csignal>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include <dirent.h>
#include <sys/time.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "global.h"
#include "SocketListener.h"


namespace   openbook {
namespace filesystem {

NotifyPipe* g_termNote;      ///< pipe used to break out of select statements


} // namespace filesystem
} // namespace openbook

using namespace openbook::filesystem;

void signal_callback( int signum )
{
    switch(signum)
    {
        case SIGINT:
        {
            std::cout << "Main thread received SIGINT, going to terminate"
                      << std::endl;
            g_termNote->notify();
            break;
        }

        default:
            std::cout << "Main thread: unexpected signal "
                      << signum <<", ignoring" << std::endl;
            break;
    }
}

int main(int argc, char** argv)
{
    // for notifiying termination
    NotifyPipe  termPipe;
    g_termNote = &termPipe;

    pthreads::Thread guiThread;
    pthreads::Thread peerThread;

    SocketListener guiListener;
    SocketListener peerListener;

    guiThread.launch(  SocketListener::start, &guiListener );
    peerThread.launch( SocketListener::start, &peerListener );

    sleep(2);
    guiListener.setInterface( "localhost", 3031 );
    peerListener.setInterface("localhost", 3032 );
    sleep(2);

    g_termNote->notify();
    guiThread.join();
    peerThread.join();

    return 0;
}

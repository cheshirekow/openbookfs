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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/Backend.h
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_BACKEND_H_
#define OPENBOOK_FS_BACKEND_H_

#include <cpp-pthreads.h>

#include "NotifyPipe.h"
#include "SocketListener.h"


namespace   openbook {
namespace filesystem {

/// encapsulates and provides communication between backend components
class Backend
{
    public:
        enum Listeners
        {
            LISTEN_GUI,
            LISTEN_PEER,
            NUM_LISTENERS,
        };

    private:
        std::string      m_configFile;  ///< configuration file to load
        NotifyPipe       m_termNote;    ///< globally signals termination

        /// listens for incoming connections
        SocketListener   m_listeners[NUM_LISTENERS];

        /// threads for listeners
        pthreads::Thread m_listenThreads[NUM_LISTENERS];

    public:
        Backend();
        ~Backend();

    private:
        /// parses the command line
        void parse( int argc, char** argv );

    public:
        /// call from main()
        int run(int argc, char** argv);
};

}
}















#endif // BACKEND_H_

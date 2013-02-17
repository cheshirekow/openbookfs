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
 *  along with gltk.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 *  @file   src/server/main.cpp
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/time.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>
#include <fstream>
#include <crypto++/files.h>
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/dh.h>
#include <crypto++/dh2.h>
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/cmac.h>
#include <crypto++/gcm.h>
#include <re2/re2.h>

#include "Bytes.h"
#include "Client.h"
#include "MessageBuffer.h"
#include "global.h"
#include "messages.h"
#include "messages.pb.h"
#include "ServerHandler.h"
#include "Queue.h"
#include "NotifyPipe.h"

namespace   openbook {
namespace filesystem {

volatile bool   g_shouldDie = false;
NotifyPipe*     g_termNote;

}
}

using namespace openbook::filesystem;

void signal_callback( int signum )
{
    switch(signum)
    {
        case SIGINT:
        {
            std::cout << "Received signal, going to terminate" << std::endl;
            g_shouldDie = true;
            g_termNote->notify();
            break;
        }

        default:
            std::cout << "Unexpected signal, ignoring" << std::endl;
            break;
    }
}

int main(int argc, char** argv)
{
    namespace fs   = boost::filesystem;
    namespace cryp = CryptoPP;
    namespace msgs = messages;

    // register signal handler
    signal(SIGINT, signal_callback);

    // initialize pipe to notify of termination
    NotifyPipe termNote;
    g_termNote = &termNote;

    std::string config;

    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

        time_t      rawtime;
        tm*         timeinfo;
        char        currentYear[5];

        ::time( &rawtime );
        timeinfo = ::localtime( &rawtime );
        strftime (currentYear,5,"%Y",timeinfo);

        fs::path homeDir     = getenv("HOME");
        fs::path dfltConfig  = "./openbookfs_c.yaml";

        std::stringstream sstream;
        sstream << "Openbook Filesystem\n"
                << "Copyright (c) 2012-" << currentYear
                << " Josh Bialkowski <jbialk@mit.edu>";

        // Define the command line object, and insert a message
        // that describes the program. The "Command description message"
        // is printed last in the help text. The second argument is the
        // delimiter (usually space) and the last one is the version number.
        // The CmdLine object parses the argv array based on the Arg objects
        // that it contains.
        TCLAP::CmdLine cmd(sstream.str().c_str(), ' ', "0.1.0");

        // Define a value argument and add it to the command line.
        // A value arg defines a flag and a type of value that it expects,
        // such as "-n Bishop".
        TCLAP::ValueArg<std::string> configArg(
                "f",
                "config",
                "path to configuration file",
                false,
                dfltConfig.string(),
                "path"
                );

        // Add the argument nameArg to the CmdLine object. The CmdLine object
        // uses this Arg to parse the command line.
        cmd.add( configArg );

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        config  = configArg.getValue();
    }

    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "error: " << e.error() << " for arg "
                    << e.argId() << std::endl;
        return 1;
    }

    // attempt to load the config file
    Client        client;

    try
    {
        client.initConfig( config );
    }
    catch( std::exception& ex )
    {
        std::cerr << "Problam loading config: " << ex.what() << std::endl;
        return 1;
    }

    ServerHandler serverHandler;
    Queue<Job*>   jobQueue;

    try
    {
        serverHandler.init(&client,&jobQueue,termNote.readFd());
        serverHandler.start();
    }
    catch( std::exception& ex )
    {
        std::cerr << "Problem starting server handler: " << ex.what() << std::endl;
        return 1;
    }

    // wait for the server to quit
    serverHandler.join();



}





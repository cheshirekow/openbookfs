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
#include <netdb.h>
#include <sys/time.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"

#include "connection.h"
#include "Options.h"
#include "ConnectOptions.h"


namespace   openbook {
namespace filesystem {

NotifyPipe* g_termNote;

} // namespace openbook
} // namespace filesystem

using namespace openbook::filesystem;
using namespace openbook::filesystem::clui;



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

// forward dec
void dispatch( int argc, char** argv, bool help = false );






std::string copyright()
{
    time_t      rawtime;
    tm*         timeinfo;
    char        currentYear[5];

    ::time( &rawtime );
    timeinfo = ::localtime( &rawtime );
    strftime (currentYear,5,"%Y",timeinfo);

    std::stringstream sstream;
    sstream << "Openbook Filesystem Command Line Interface\n"
            << "Copyright (c) 2012-" << currentYear
            << " Josh Bialkowski <jbialk@mit.edu>\n";

    return sstream.str();
}

void print_usage(const char* argv0 = 0 )
{
    std::string commandName = argv0 ? argv0 : "obfs";
    std::cout << copyright();
    std::cout << "Usage: " << commandName << " <command> "
              << "[command options]\n\n";
    std::cout << "command may be any of:"
        "\n help [command]   print help for any of the following commands"
        "\n connect          force connection attempt to a peer"
        "\n set              set a configuration variable"
        "\n";
}


template <typename Options_t>
void parse_and_go(int argc, char** argv, bool help=false)
{
    // Define the command line object, and insert a message
    // that describes the program. The "Command description message"
    // is printed last in the help text. The second argument is the
    // delimiter (usually space) and the last one is the version number.
    // The CmdLine object parses the argv array based on the Arg objects
    // that it contains.
    CmdLine cmd( argv[0], copyright(), ' ', "0.1.0");
    Options_t opts(cmd);

    // Parse the argv array.
    // if we just need the help then
    if( help )
    {
        TCLAP::StdOutput out;
        out.usage(cmd);
        return;
    }

    cmd.parse( argc, argv );
    opts.go();
}


void dispatch( int argc, char** argv, bool help )
{
   std::string cmd = argc > 0 ? argv[0] : "usage";
   if( cmd == "help" )
   {
       argc--;
       argv++;
       dispatch(argc,argv,true);
   }
   else if( cmd == "connect" )
        parse_and_go<ConnectOptions>(argc,argv,help);
   else if( cmd == "usage" )
        print_usage();
   else
   {
       std::cout << "unrecognized command:" << cmd << "\n";
       print_usage();
   }
}

int main(int argc, char** argv)
{
    // for cancellation
    NotifyPipe termNote;
    g_termNote = &termNote;

    // install signal handlers
    signal(SIGINT,signal_callback);
    signal(SIGPIPE,signal_callback);

    // we need at least two arguments, command + subcommand
    if( argc < 2 )
    {
        std::cout << "not enough parameters:\n";
        print_usage(argv[0]);
        return 0;
    }

    // parse commands and dispatch appropriate function
    argc -= 1;
    argv += 1;

    try
    {
        dispatch(argc,argv);
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception in main:" << ex.what() << "\n";
    }

    return 0;
}

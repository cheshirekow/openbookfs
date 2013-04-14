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
void dispatch( const std::string& cmd, int argc, char** argv,
                bool help = false );









void do_connect(ConnectOptions& )
{

}













std::string copyright()
{
    time_t      rawtime;
    tm*         timeinfo;
    char        currentYear[5];

    ::time( &rawtime );
    timeinfo = ::localtime( &rawtime );
    strftime (currentYear,5,"%Y",timeinfo);

    std::stringstream sstream;
    sstream << "Openbook Filesystem Backend\n"
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


/// print help for specified arguments
void parse_help(int argc, char** argv)
{
    if(argc < 1)
    {
        print_usage();
        return;
    }

    // get the command that we wish to to get the help for
    std::string subcmd = argv[0];
    argc -= 1;
    argv += 1;

    // redispatch with help set to true
    dispatch(subcmd,argc,argv,true);
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
    TCLAP::CmdLine cmd( copyright(), ' ', "0.1.0");
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


void dispatch( const std::string& cmd, int argc, char** argv, bool help )
{
   if( cmd == "help" )
        parse_help(argc,argv);
   else if( cmd == "connect" )
        parse_and_go<ConnectOptions>(argc,argv);
   else
        print_usage();
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
        print_usage(argv[0]);
        return 0;
    }

    // parse commands and dispatch appropriate function
    std::string subcmd = argv[0];
    argc -= 1;
    argv += 1;

    try
    {
        dispatch(subcmd,argc,argv);
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception in main:" << ex.what() << "\n";
    }

    return 0;
}

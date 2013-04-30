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


#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"

#include "connection.h"
#include "Options.h"
#include "commands/Connect.h"
#include "commands/SetClientSocket.h"
#include "commands/SetDataDir.h"
#include "commands/SetDisplayName.h"
#include "commands/ListKnownPeers.h"
#include "commands/ListMounts.h"
#include "commands/LoadConfig.h"
#include "commands/SetRemoteSocket.h"


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

void print_usage(const char* argv0 = 0 );



/// parses a command and then executes it
/**
 *  @tparam Options_t   derived class of Options which implements the
 *                      command line parser and has a method go() which
 *                      actually performs the command
 *  @param  argc        number of arguments
 *  @param  argv        argument vector
 *  @param  help        if true, dont do actual work, simply print usage
 *                      for the subcommand
 */
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

    // if we just need the help then print usage/help for the subcommand
    if( help )
    {
        TCLAP::StdOutput out;
        out.usage(cmd);
    }
    else
    {
        // otherwise parse the command line into the Options_t object
        // and then run the command
        cmd.parse( argc, argv );
        opts.go();
    }
}


void printPair( const std::string& cmd, const std::string& desc,
                int fieldWidth, int indent )
{
    int nSpaces = 3+fieldWidth - (cmd.size() + indent);

    for(int i=0; i < indent; i++)
        std::cout << " ";
    std::cout << cmd;
    for(int i=0; i < nSpaces; i++)
        std::cout << " ";
    std::cout << desc << "\n";
}


template < typename ...TList >
struct DispatchList;

template < typename TFirst >
struct DispatchList< TFirst >
{
    static bool dispatch( const std::string& cmd,
                            int argc, char** argv, bool help )
    {
        if( cmd == TFirst::COMMAND )
        {
            parse_and_go<TFirst>(argc,argv,help);
            return true;
        }
        return false;
    }

    static std::size_t getFieldWidth()
    {
        return TFirst::COMMAND.size();
    }

    static void printUsage(std::size_t fieldWidth, std::size_t indent )
    {
        printPair( TFirst::COMMAND, TFirst::DESCRIPTION, fieldWidth, indent );
    }
};

template < typename TFirst, typename ...TRest >
struct DispatchList<TFirst,TRest...>
{
    static bool dispatch( const std::string& cmd,
                            int argc, char** argv, bool help )
    {
        if( DispatchList<TFirst>::dispatch(cmd,argc,argv,help) )
            return true;
        if( DispatchList<TRest...>::dispatch(cmd,argc,argv,help) )
            return true;
        return false;
    }

    static std::size_t getFieldWidth()
    {
        return std::max(
                    DispatchList<TFirst>::getFieldWidth(),
                    DispatchList<TRest...>::getFieldWidth() );
    }

    static void printUsage(std::size_t fieldWidth, std::size_t indent )
    {
        DispatchList<TFirst>::printUsage(fieldWidth,indent);
        DispatchList<TRest...>::printUsage(fieldWidth,indent);
    }
};

typedef DispatchList< Connect, 
                      LoadConfig >      SingleCommands;

typedef DispatchList< ListKnownPeers,
                      ListMounts >      ListCommands;

typedef DispatchList< SetDataDir,
                      SetDisplayName,
                      SetClientSocket,
                      SetRemoteSocket>  SetCommands;


void print_usage(const char* argv0 )
{
    std::string commandName = argv0 ? argv0 : "obfs";
    std::cout << copyright();
    std::cout << "Usage: " << commandName << " <command> "
              << "[command options]\n\n";

    std::size_t fieldWidth=0;
    std::size_t fieldWidth0 = SingleCommands::getFieldWidth();
    std::size_t fieldWidth1 = 0;
    fieldWidth1 = std::max( fieldWidth1, ListCommands::getFieldWidth() );
    fieldWidth1 = std::max( fieldWidth1, SetCommands::getFieldWidth() );

    fieldWidth = std::max( fieldWidth0, fieldWidth1+3 );

    const char* staticCmd[] =
    {
        "help [command]",
           "print help for any of the following commands",
        "ls [arg]",
           "print a list of [arg] (see below) ",
        "set [arg] [value]",
           "set a backend configuration variable (see below) ",
        0
    };

    for(const char** cmd = staticCmd; *cmd; cmd+=2 )
        fieldWidth = std::max( fieldWidth, std::string(*cmd).length() );

    std::cout << "command may be any of:\n";
    printPair( staticCmd[0], staticCmd[1], fieldWidth, 0 );

    SingleCommands::printUsage(fieldWidth,0);
    printPair( staticCmd[2], staticCmd[3], fieldWidth, 0 );
    ListCommands::printUsage(fieldWidth,3);
    printPair( staticCmd[4], staticCmd[5], fieldWidth, 0 );
    SetCommands::printUsage(fieldWidth,3);

}



/// parses the first argument of argv (the subcommand name) and then calls
/// the apropriate function to actually do the work
/**
 *  @param argc number of arguments
 *  @param argv argument vector, argv[0] is subcommand name
 *  @param help if true, don't do any actual work but instead just print the
 *         help for the subcommand
 *
 *  todo: Bruno, you may want to consider making "set" a subcommand which
 *        takes an additional argument being the variable name, instead of
 *        having separate commands for set_displayName, set_clientInterface,
 *        etc.... If you choose to go this route you may want a separate
 *        dispatch function which looks just like this one but switches on
 *        variable names
 */
void dispatch( int argc, char** argv, bool help )
{
   std::string cmd = argc > 0 ? argv[0] : "usage";

   // if the command is help, we simply pop off the first argv (the "help"
   // string) and then recurse on dispatch but with help set to true
   if( cmd == "help" )
   {
       argc--;
       argv++;
       dispatch(argc,argv,true);
   }
   else if( cmd == "usage" )
        print_usage();
   else if( cmd == "set" )
   {
       argc--;
       argv++;
       std::string cmd = argc > 0 ? argv[0] : "usage";
       if( !SetCommands::dispatch(cmd,argc,argv,help) )
       {
           std::cout << "unrecognized set command:" << cmd << "\n";
           print_usage();
       }
   }
   else if( cmd == "ls" )
   {
       argc--;
       argv++;
       std::string cmd = argc > 0 ? argv[0] : "usage";
       if( !ListCommands::dispatch(cmd,argc,argv,help) )
       {
           std::cout << "unrecognized ls command:" << cmd << "\n";
           print_usage();
       }
   }
   else
   {
       std::cout << "unrecognized command:" << cmd << "\n";
       print_usage();
   }
}


/// main entry point into the command line user interface
/**
 *  usage:
 *      obfs <command> [command options]
 */
int main(int argc, char** argv)
{
    // for cancellation, opens an unnamed pipe that can be select()'ed to
    // preempt any socket calls
    NotifyPipe termNote;
    g_termNote = &termNote;

    // install signal handlers, when SIGINT is signalled (i.e. ctrl+c in the
    // terminal) the termNote is signalled any any blocking network calls
    // will break out, allowing the program to terminate
    signal(SIGINT,signal_callback);
    signal(SIGPIPE,signal_callback);

    // we need at least two arguments, command + subcommand
    if( argc < 2 )
    {
        std::cout << "not enough parameters:\n";
        print_usage(argv[0]);
        return 0;
    }

    // argv[0] is the name of the command that started this program, so we
    // advance the argv pointer to the next argument and reduce the argument
    // count by one
    argc -= 1;
    argv += 1;

    try
    {
        // dispatch switches on the subcommand and calls the appropriate
        // function
        dispatch(argc,argv);
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception in main:" << ex.what() << "\n";
    }

    return 0;
}

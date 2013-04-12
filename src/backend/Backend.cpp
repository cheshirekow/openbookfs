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

#include <ctime>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "fuse_include.h"
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

const std::string& Backend::publicKey()
{
    return m_pubKey;
}

int Backend::connectPeer( const std::string& publickKey )
{
    return 0;
}

void Backend::disconnectPeer( int peerId )
{

}

std::string Backend::privateKeyFile()
{
    return "***noKeyFileSet***";
}

void Backend::parse(int argc, char** argv)
{
    namespace fs = boost::filesystem;

    bool        fuseHelp    = false;
    bool        fuseVersion = false;

    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {
        time_t      rawtime;
        tm*         timeinfo;
        char        currentYear[5];

        ::time( &rawtime );
        timeinfo = ::localtime( &rawtime );
        strftime (currentYear,5,"%Y",timeinfo);

        std::stringstream sstream;
        sstream << "Openbook Filesystem Backend\n"
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
                "./obfs.yaml",
                "path"
                );

        TCLAP::SwitchArg fuseHelpArg(
                "H",
                "fusehelp",
                "show help output and options from fuse",
                false
                );

        TCLAP::SwitchArg fuseVersionArg(
                "V",
                "fuseversion",
                "show version info from fuse",
                false
                );

        // Add the argument nameArg to the CmdLine object. The CmdLine object
        // uses this Arg to parse the command line.
        cmd.add( configArg );
        cmd.add( fuseHelpArg );
        cmd.add( fuseVersionArg );

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        m_configFile = configArg.getValue();
        fuseHelp     = fuseHelpArg.getValue();
        fuseVersion  = fuseVersionArg.getValue();
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "Argument error: " << e.error() << " for arg "
                    << e.argId() << std::endl;
    }

    if( fuseHelp )
    {
        char* argv[] = {(char*)"", (char*)"-h"};
        fuse_main(2,argv,(fuse_operations*)0,(void*)0);
        exit(0);
    }

    if( fuseVersion )
    {
        char* argv[] = {(char*)"", (char*)"-V"};
        fuse_main(2,argv,(fuse_operations*)0,(void*)0);
        exit(0);
    }
}

int Backend::run(int argc, char** argv)
{
    parse(argc, argv);

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

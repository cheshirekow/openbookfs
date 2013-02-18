/*
 * main.cpp
 *
 *  Created on: Jun 7, 2012
 *      Author: josh
 */

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

#include "fuse_include.h"
#include "fuse_operations.h"
#include "OpenbookFS.h"
#include "Bytes.h"
#include "Client.h"
#include "MessageBuffer.h"
#include "global.h"
#include "messages.h"
#include "messages.pb.h"
#include "ServerHandler.h"
#include "Pool.h"
#include "JobHandler.h"
#include "Queue.h"
#include "NotifyPipe.h"
#include "jobs/QuitWorker.h"

using namespace openbook::filesystem;

namespace   openbook {
namespace filesystem {

volatile bool   g_shouldDie = false;
NotifyPipe*     g_termNote;

}
}

int main(int argc, char** argv)
{
    namespace fs = boost::filesystem;

    // location of the configuration file
    std::string     configFile;

    // Generate a command line string for fuse
    std::list<std::string>  fuse_argv_list;

    // indicates that help or version was requested so dont do anything
    // else
    bool helpOrVersion = false;

    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try
    {
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
                << " Josh Bialkowski <josh.bialkowski@gmail.com>";

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
        // Define a value argument and add it to the command line.
        // A value arg defines a flag and a type of value that it expects,
        // such as "-n Bishop".
        TCLAP::ValueArg<std::string> configArg(
                "F",
                "config",
                "path to configuration file",
                false,
                dfltConfig.string(),
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

        TCLAP::SwitchArg debugArg(
                "d",
                "debug",
                "enable debug output (implies -f)",
                false
                );

        TCLAP::SwitchArg foregroundArg(
                "f",
                "foreground",
                "foreground operation (don't daemonize)",
                false
                );

        TCLAP::SwitchArg singlethreadArg(
                "s",
                "singlethread",
                "disable multi-threaded operation",
                false
                );

        TCLAP::MultiArg<std::string> optionArgs(
                "o",
                "options",
                "additional options for fuse (--fusehelp for more info)",
                false,
                "strings"
                );


        // Add the argument nameArg to the CmdLine object. The CmdLine object
        // uses this Arg to parse the command line.
        cmd.add( configArg );
        cmd.add( fuseHelpArg );
        cmd.add( fuseVersionArg );
        cmd.add( debugArg );
        cmd.add( foregroundArg );
        cmd.add( singlethreadArg );
        cmd.add( optionArgs );

        // Parse the argv array.
        cmd.parse( argc, argv );

        configFile = configArg.getValue();

        // turn real command line arguments into arguments for fuse
        fuse_argv_list.push_back(cmd.getProgramName());

        if( fuseHelpArg.getValue() || fuseVersionArg.getValue() )
            helpOrVersion = true;

        if(fuseHelpArg.getValue())
            fuse_argv_list.push_back("-h");

        if(fuseVersionArg.getValue())
            fuse_argv_list.push_back("-V");

        if(debugArg.getValue())
            fuse_argv_list.push_back("-d");

        if(foregroundArg.getValue())
            fuse_argv_list.push_back("-f");

        if(singlethreadArg.getValue())
            fuse_argv_list.push_back("-s");

        for(int i=0; i < optionArgs.getValue().size(); i++)
        {
            fuse_argv_list.push_back("-o");
            fuse_argv_list.push_back( optionArgs.getValue()[i] );
        }
    }

    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "Error parsing command line: "
                    << e.error() << " for arg "
                    << e.argId() << std::endl;
        return 1;
    }

    // attempt to load the config file
    Client        client;


    try
    {
        if( !helpOrVersion )
            client.initConfig( configFile );
    }
    catch( std::exception& ex )
    {
        std::cerr << "Problam loading config: " << ex.what() << std::endl;
        return 1;
    }

    // set the fields of the fs initialization structure
    OpenbookFS_Init fs_init;
    fs_init.client = &client;

    // if we're not a help or version call, append the mount point to
    // fuse options
    if( !helpOrVersion )
    {
        std::list<std::string>::iterator itArgv = fuse_argv_list.begin();
        fuse_argv_list.insert( ++itArgv, client.rootDir() );
    }

    // calculate the number of bytes we need to store argv
    std::list<std::string>::iterator    itArgv;
    int                                 nChars = 0;
    for(itArgv = fuse_argv_list.begin();
            itArgv != fuse_argv_list.end(); itArgv++)
        nChars += itArgv->length() + 1;

    // allocate such a character array
    char*   argv_buf  = new char[nChars];
    int     iArg      = 0;
    int     iBuf      = 0;
    int     fuse_argc = fuse_argv_list.size();
    char**  fuse_argv = new char*[fuse_argc];

    for(itArgv = fuse_argv_list.begin();
            itArgv != fuse_argv_list.end(); itArgv++)
    {
        char*   ptrArg      = argv_buf + iBuf;
        int     argLen      = itArgv->length();
        fuse_argv[iArg++] = ptrArg;

        itArgv->copy(ptrArg,argLen);
        iBuf += argLen;

        argv_buf[iBuf] = '\0';
        iBuf ++;
    }

    std::cerr << "Finished building argument vector: \n   ";
    for(int i=0; i < nChars; i++)
    {
        if(argv_buf[i] != '\0')
            std::cerr << argv_buf[i];
        else
            std::cerr << " ";
    }
    std::cerr << std::endl;

    umask(0);

    // set the fuse_ops structure
    fuse_operations fuse_ops;
    setFuseOps(fuse_ops);

    int fuseResult = 0;
    if( helpOrVersion)
        fuseResult = fuse_main(fuse_argc, fuse_argv, &fuse_ops, 0);
    else
        fuseResult = fuse_main(fuse_argc, fuse_argv, &fuse_ops, &fs_init);
    g_shouldDie = true;

    delete [] argv_buf;
    delete [] fuse_argv;

    return fuseResult;
}

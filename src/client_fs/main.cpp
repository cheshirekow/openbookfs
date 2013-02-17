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

using namespace openbook::filesystem;

int main(int argc, char** argv)
{
    namespace fs = boost::filesystem;

    fuse_operations fuse_ops;
    setFuseOps(fuse_ops);

    int             fuse_argc;
    char**          fuse_argv;
    OpenbookFS_Init fs_init;

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
        fs::path dfltDataDir = homeDir / ".openbook/data";
        fs::path dfltMountDir= homeDir / "openbook";


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
        TCLAP::ValueArg<std::string> dataDirArg(
                "a",    // ......................................... short flag
                "data", // .......................................... long flag
                "data directory on the "    // ...... user-readable description
                    "actual file system",
                false,   // ......................................... required?
                dfltDataDir.string(),   // ...................... default value
                "path"  // ................................. user readable type
                );

        TCLAP::UnlabeledValueArg<std::string> mountArg(
                "mount_point",  // ....................................... name
                "where to mount the filesystem ",  // user-readable description
                false,   // ......................................... required?
                dfltMountDir.string(),   // ..................... default value
                "path"  // ................................. user readable type
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
        cmd.add( mountArg );
        cmd.add( dataDirArg );
        cmd.add( fuseHelpArg );
        cmd.add( fuseVersionArg );
        cmd.add( debugArg );
        cmd.add( foregroundArg );
        cmd.add( singlethreadArg );
        cmd.add( optionArgs );

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        fs_init.dataDir = dataDirArg.getValue();

        // Generate a command line string for fuse
        std::list<std::string>  fuse_argv_list;

        fuse_argv_list.push_back(cmd.getProgramName());

        if( !( fuseHelpArg.getValue() || fuseVersionArg.getValue() ) )
            fuse_argv_list.push_back(mountArg.getValue());

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

        // calculate the number of bytes we need to store argv
        std::list<std::string>::iterator    itArgv;
        int                                 nChars = 0;
        for(itArgv = fuse_argv_list.begin();
                itArgv != fuse_argv_list.end(); itArgv++)
            nChars += itArgv->length() + 1;

        // allocate such a character array
        char* argv_buf  = new char[nChars];
        int   iArg      = 0;
        int   iBuf      = 0;
        fuse_argc   = fuse_argv_list.size();
        fuse_argv   = new char*[fuse_argc];

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

    }

    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "error: " << e.error() << " for arg "
                    << e.argId() << std::endl;
        return 1;
    }

    umask(0);

    int fuseResult = fuse_main(fuse_argc, fuse_argv, &fuse_ops, &fs_init);
    return fuseResult;
}

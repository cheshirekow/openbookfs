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


#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <iostream>
#include <cpp-pthreads.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include <crypto++/files.h>
#include <crypto++/rsa.h>
#include <crypto++/rng.h>

#include "Pool.h"
#include "Bytes.h"
#include "RequestHandler.h"
#include "Server.h"

using namespace openbook::filesystem;


int  g_pipefd[2]; ///< pipe for waking up listener when signaled

void signal_callback( int signum )
{
    switch(signum)
    {
        case SIGINT:
        {
            std::cout << "Received signal, going to terminate" << std::endl;
            write(g_pipefd[1],"x",1);
            break;
        }

        default:
            std::cout << "Unexpected signal, ignoring" << std::endl;
            break;
    }
}


int main(int argc, char** argv)
{
    // register signal handler
    signal(SIGINT, signal_callback);

    // create a pipe we can signal
    if( pipe(g_pipefd) )
    {
        std::cerr << "Can't create pipe" << std::endl;
        return 1;
    }


    namespace fs = boost::filesystem;

    std::string configFile;
    std::string pubKey;
    std::string privKey;
    std::string dataDir;
    int         port;

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
    fs::path dfltConfig  = "./openbookfs_d.yaml";
    fs::path dfltDataDir = "./data";
    fs::path dfltPubKey  = "./rsa-openssl-pub.der";
    fs::path dfltPrivKey = "./rsa-openssl-priv.der";
    int      dfltPort    = 3031;


    std::stringstream sstream;
    sstream << "Openbook Filesystem Server\n"
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

    TCLAP::ValueArg<std::string> pubKeyArg(
            "b",
            "pubkey",
            "path to the ssh public key",
            false,
            dfltPubKey.string(),
            "path"
            );

    TCLAP::ValueArg<std::string> privKeyArg(
            "v",
            "privkey",
            "path to the ssh private key",
            false,
            dfltPrivKey.string(),
            "path"
            );

    TCLAP::ValueArg<std::string> dataDirArg(
            "d",
            "data",
            "path to root of file system",
            false,
            dfltDataDir.string(),
            "path"
            );

    TCLAP::ValueArg<int> portArg(
            "p",
            "port",
            "port to listen on",
            false,
            dfltPort,
            "integer"
            );

    // Add the argument nameArg to the CmdLine object. The CmdLine object
    // uses this Arg to parse the command line.
    cmd.add( pubKeyArg );
    cmd.add( privKeyArg );
    cmd.add( dataDirArg );
    cmd.add( portArg );

    // Parse the argv array.
    cmd.parse( argc, argv );

    // Get the value parsed by each arg.
    configFile = configArg.getValue();
    pubKey     = pubKeyArg.getValue();
    privKey    = privKeyArg.getValue();
    dataDir    = dataDirArg.getValue();
    port       = portArg.getValue();

    }

    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "error: " << e.error() << " for arg "
                    << e.argId() << std::endl;
        return 1;
    }

    Server server;

    try
    {
        server.initData( dataDir );
        server.initConfig( configFile );
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception while initializing server data: "
                  << ex.what() << std::endl;
        return 1;
    }

    int serversock, clientsock;
    struct sockaddr_in server_in, client_in;

    //  Create the TCP socket
    serversock = socket(PF_INET, SOCK_STREAM | O_NONBLOCK, IPPROTO_TCP);
    if (serversock < 0)
    {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // So that we can re-bind to it without TIME_WAIT problems
    int reuse_addr = 1;
    setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR,
                    &reuse_addr, sizeof(reuse_addr));

    //  Construct the server sockaddr_in structure
    memset(&server_in, 0, sizeof(server_in));       //  Clear struct
    server_in.sin_family       = AF_INET;            //  Internet/IP
    server_in.sin_addr.s_addr  = htonl(INADDR_ANY);  //  Incoming addr
    server_in.sin_port         = htons(port);        //  server port

    //  Bind the server socket
    if (bind(serversock, (struct sockaddr *) &server_in, sizeof(server_in)) < 0)
    {
        std::cerr << "Failed to bind the server socket\n";
        return 1;
    }

    //  Listen on the server socket, 10 max pending connections
    if (listen(serversock, 10) < 0)
    {
        std::cerr << "Failed to listen on server socket\n";
        return 1;
    }


    // Pool of request handlers, 20 handlers
    std::cout << "Initializing handler pool" << std::endl;
    Pool<RequestHandler> handlerPool(5);

    // for selecting
    fd_set selectMe;


    //  Run until cancelled
    while (1)
    {
        FD_ZERO( &selectMe );   //< zero out the select struct
        FD_SET(g_pipefd[0], &selectMe ); //< add pipe to our fdset
        FD_SET(serversock,  &selectMe ); //< add server socket to our fd set


        // wait for something to happen
        timeval tv_timeout = {5,0};
        int maxfd     = std::max(g_pipefd[0], serversock) + 1;
        int numChange = select(maxfd,&selectMe, 0, 0, &tv_timeout );
        std::cout << "Waking up" << std::endl;

        if( FD_ISSET(g_pipefd[0], &selectMe ) )
        {
            std::cout << "Terminating!!" << std::endl;
            break;
        }
//        else if( !FD_ISSET(serversock, &selectMe) )
//        {
//            std::cout << "Woke up but don't know why, must be timeout" << std::endl;
//            continue;
//        }

        unsigned int clientlen = sizeof(client_in);

        //  Wait for client connection (should not block)
        clientsock = accept(
                serversock,
                (struct sockaddr *) &client_in,
                &clientlen );

        if( clientsock < 0 && errno == EWOULDBLOCK )
        {
            std::cerr << "No pending connections" << std::endl;
            continue;
        }
        else if (clientsock < 0)
        {
            std::cerr << "Failed to accept client connection: "
                      << clientsock << "\n";
            return 1;
        }

        Bytes<in_addr_t> ip( &client_in.sin_addr.s_addr );

        std::cout << "Client connected: "
                  << ip[0] << "."
                  << ip[1] << "."
                  << ip[2] << "."
                  << ip[3] << "\n";

        // set a timeout on the socket
        timeval tv = {2,0};
        int optResult = setsockopt(clientsock, SOL_SOCKET,
                            SO_RCVTIMEO, (char *)&tv,  sizeof tv);
        if( optResult < 0 )
        {
            std::cerr << "Failed to set timeout on client sock"
                      << std::endl;
            return 1;
        }

        // get a request handler
        RequestHandler* handler = handlerPool.getAvailable();

        if(handler)
            handler->start(clientsock);
        else
        {
            std::cerr << "no available handlers, terminating connection"
                      << std::endl;
            close(clientsock);
        }


    }
}





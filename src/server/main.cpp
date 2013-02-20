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


#include <iostream>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <set>

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cpp-pthreads.h>
#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include <crypto++/files.h>
#include <crypto++/rsa.h>
#include <crypto++/rng.h>

#include "global.h"
#include "ClientHandler.h"
#include "ClientMap.h"
#include "NotifyPipe.h"
#include "Pool.h"
#include "MessageHandler.h"
#include "Queue.h"
#include "Server.h"
#include "SelectSpec.h"
#include "Synchronized.h"


namespace   openbook {
namespace filesystem {
 namespace    server {

// declarted in globa.h
pthreads::Key g_handlerKey;
NotifyPipe* g_termNote;

} // namespace server
} // namespace filesystem
} // namespace openbook


using namespace openbook::filesystem;
using namespace server;

void signal_callback( int signum )
{
    switch(signum)
    {
        case SIGINT:
        {
            std::cout << "Received signal, going to terminate" << std::endl;
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
    // register signal handler
    signal(SIGINT, signal_callback);

    // initialize pipe to notify of termination
    NotifyPipe termNote;
    g_termNote = &termNote;

    namespace fs = boost::filesystem;

    std::string configFile;

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

        // Add the argument nameArg to the CmdLine object. The CmdLine object
        // uses this Arg to parse the command line.
        cmd.add( configArg );

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        configFile = configArg.getValue();
    }

    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "Argument error: " << e.error() << " for arg "
                    << e.argId() << std::endl;
        return 1;
    }

    Server server;

    try
    {
        server.initConfig( configFile );
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception while initializing server data: "
                  << ex.what() << std::endl;
        return 1;
    }

    int serversock, clientsock;

    try
    {
        // defaults
        addrinfo  hints;
        addrinfo* found;
        memset(&hints,0,sizeof(addrinfo));
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if( server.addressFamily() == "AF_INET" )
            hints.ai_family = AF_INET;
        else if( server.addressFamily() == "AF_INET6" )
            hints.ai_family = AF_INET6;
        else if( server.addressFamily() != "AF_UNSPEC" )
        {
            std::cerr << "I dont understand the address family "
                      << server.addressFamily() << ", check the config file. "
                      << "I will use AF_UNSPEC to seach for an interface";
        }

        const char* node = 0;
        if( server.iface() == "any" )
            hints.ai_flags |= AI_PASSIVE;
        else
            node = server.iface().c_str();

        const char* service = server.port().c_str();

        int result = getaddrinfo(node,service,&hints,&found);
        if( result < 0 )
        {
            ex()() << "Failed to find an interface which matches family: "
                   << server.addressFamily() << ", node: "
                   << server.iface() << ", service: " << server.port()
                   << "\nErrno is " << errno << " : " << strerror(errno);
        }

        addrinfo* addr = found;

        for( ; addr; addr = addr->ai_next )
        {
            std::cout << "Attempting to create socket:"
                      << "\n   family: " << addr->ai_family
                      << "\n     type: " << addr->ai_socktype
                      << "\n protocol: " << addr->ai_protocol
                      << std::endl;

            serversock = socket(addr->ai_family,
                                addr->ai_socktype | O_NONBLOCK,
                                addr->ai_protocol);
            if (serversock < 0)
                continue;
            else
                break;
        }

        if( !addr )
            ex()() << "None of the returned addresses work";

        // So that we can re-bind to it without TIME_WAIT problems
        int reuse_addr = 1;
        if( setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR,
                            &reuse_addr, sizeof(reuse_addr)) )
            ex()() << "Failed to set SO_REUSEADDR on socket";

        //  Bind the server socket
        if ( bind(serversock, addr->ai_addr, addr->ai_addrlen) < 0)
            ex()() << "Failed to bind the server socket\n";

        char host[NI_MAXHOST];
        char port[NI_MAXSERV];
        memset(host, 0, sizeof(host));
        memset(port, 0, sizeof(port));
        getnameinfo( (sockaddr*)addr->ai_addr, addr->ai_addrlen,
                     host, sizeof(host),
                     port, sizeof(port),
                     NI_NUMERICHOST | NI_NUMERICSERV );

        std::cout << "Bound server to " << host << ":" << port << std::endl;

        //  Listen on the server socket, 10 max pending connections
        if (listen(serversock, 10) < 0)
        ex()() << "Failed to listen on server socket\n";

        freeaddrinfo(found);
    }
    catch( std::exception& ex )
    {
        std::cerr << "Error while setting up the socket: "
                  << ex.what() << std::endl;
        if( serversock > 0 )
            close(serversock);
        return 1;
    }


    // Pool of request handlers
    int nH = server.maxConn();
    int nJ = server.maxWorkers();
    std::cout << "Initializing handler pool (" << nH << "," << nJ << ")"
              << std::endl;

    // maps client ids to the handler object
    ClientMap   clientMap;

    // handlers available for new client connections
    ClientHandler::Pool_t      handlerPool(nH);
    MessageHandler::Pool_t     workerPool(nJ);

    // message queue
    Queue<ClientMessage>    inboundQueue;

    // allows us to store the client handler pointer in thread specific
    // storage
    g_handlerKey.create();

    // handler objects
    ClientHandler*  handlers = new ClientHandler[nH];
    MessageHandler* workers  = new MessageHandler[nJ];

    for(int i=0; i < nH; i++)
        handlers[i].init(&handlerPool,&server,&inboundQueue,&clientMap);

    for(int i=0; i < nJ; i++)
    {
        workers[i].init(&workerPool,&inboundQueue,&server,&clientMap);
        workers[i].start();
    }

    // for waiting until things happen
    SelectSpec selectMe;
    {
        using namespace select_spec;
        selectMe.gen()( termNote.readFd(), READ )
                      ( serversock, READ )
                      ( TimeVal(5,0 ) );
    }

    //  Run until cancelled
    try
    {
        bool shouldQuit = false;
        while (!shouldQuit)
        {
            // wait for something to happen
            if( !selectMe.wait() )
            {
                std::cout << "Who dares disturb my slumber. zzz..." << std::endl;
                continue;
            }
            else if( selectMe.ready(termNote.readFd(),select_spec::READ) )
            {
                std::cout << "Terminating!!" << std::endl;
                break;
            }

            std::cout << "Woke up to accept a connection" << std::endl;
            sockaddr_storage clientaddr;
            unsigned int     addrlen  = sizeof(sockaddr_storage);
            char clienthost[NI_MAXHOST];
            char clientport[NI_MAXSERV];

            //  Wait for client connection (should not block)
            clientsock = accept4(
                    serversock,
                    (sockaddr*)&clientaddr,
                    &addrlen,
                    SOCK_NONBLOCK);

            if( clientsock < 0 )
            {
                switch( errno )
                {
                    case EWOULDBLOCK:
                    {
                        std::cout << "No pending connections" << std::endl;
                        continue;
                    }

                    default:
                    {
                        ex()() << "Failed to accept client connection: errno "
                               << errno << " : " << strerror(errno) ;
                        break;
                    }
                }

            }


            memset(clienthost, 0, sizeof(clienthost));
            memset(clientport, 0, sizeof(clientport));
            getnameinfo( (sockaddr*)&clientaddr, addrlen,
                         clienthost, sizeof(clienthost),
                         clientport, sizeof(clientport),
                         NI_NUMERICHOST | NI_NUMERICSERV );

            std::cout << "Client connected host=[" << clienthost
                      << "] port=[" << clientport << "]" << std::endl;

            // get a request handler
            ClientHandler* handler = handlerPool.getAvailable();

            if(handler)
                handler->handleClient(clientsock, termNote.readFd());
            else
            {
                std::cout << "no available handlers, terminating connection"
                          << std::endl;
                close(clientsock);
            }
        }
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception in main loop: " << ex.what() << std::endl;
    }

    // wait for all the child threads to quit
    std::cout << "Waiting for client handlers to terminate" << std::endl;

    // wait until the handler pool is full
    while( handlerPool.size() < nH )
    {
        std::cout << "Waiting for " << nH - handlerPool.size()
                  << " more threads to finish " << std::endl;
        sleep(5);
    }

    // now we need to kill all the job workers
    std::cout << "Killing job workers" << std::endl;
    ClientMessage quit(0,0,MSG_QUIT);
    for(int i=0; i < nJ; i++)
        inboundQueue.insert(quit);

    while( workerPool.size() < nJ )
    {
        std::cout << "Waiting for " << nJ - workerPool.size()
                  << " more threads to finish " << std::endl;
        sleep(5);
    }

    // now destroy any extra jobs that we created for the workers
    std::cout << "Destroying outstanding jobs" << std::endl;
    while( !inboundQueue.empty() )
    {
        ClientMessage msg;
        inboundQueue.extract(msg);
        if(msg.typed.msg)
            delete msg.typed.msg;
    }

    std::cout << "Final destructions" << std::endl;
    g_handlerKey.destroy();
    delete [] handlers;
    delete [] workers;
    close(serversock);

    std::cout << "All cleaned up, terminating" << std::endl;
}





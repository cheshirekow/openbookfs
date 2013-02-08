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


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <cpp-pthreads.h>

#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 32

void HandleClient(int sock)
{
    char buffer[BUFFSIZE];
    int received = -1;
    /* Receive message */
    if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0)
    {
        std::cerr << "Failed to receive initial bytes from client\n";
        exit(1);
    }
    /* Send bytes and check for more incoming data in loop */
    while (received > 0)
    {
        /* Send back received data */
        if (send(sock, buffer, received, 0) != received)
        {
            std::cerr << "Failed to send bytes to client\n";
            exit(1);
        }

        /* Check for more data */
        if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0)
        {
            std::cerr << "Failed to receive additional bytes from client\n";
            exit(1);
        }
    }
    close(sock);
}

int main(int argc, char** argv)
{
    int serversock, clientsock;
    struct sockaddr_in echoserver, echoclient;

    if (argc != 2)
    {
        std::cerr << "USAGE: echoserver <port>\n";
        exit(1);
    }

    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        std::cerr << "Failed to create socket\n";
        return 1;
    }
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
    echoserver.sin_port = htons(atoi(argv[1]));       /* server port */

    /* Bind the server socket */
    if (bind(serversock, (struct sockaddr *) &echoserver,
    sizeof(echoserver)) < 0)
    {
        std::cerr << "Failed to bind the server socket\n";
        return 1;
    }
    /* Listen on the server socket */
    if (listen(serversock, MAXPENDING) < 0)
    {
        std::cerr << "Failed to listen on server socket\n";
    }

    /* Run until cancelled */
    while (1)
    {
        unsigned int clientlen = sizeof(echoclient);
        /* Wait for client connection */
        clientsock = accept(
                serversock,
                (struct sockaddr *) &echoclient,
                &clientlen );
        if (clientsock < 0)
        {
            std::cerr << "Failed to accept client connection\n";
            return 1;
        }
        fprintf(stdout, "Client connected: %s\n",
                    inet_ntoa(echoclient.sin_addr));
        HandleClient(clientsock);
    }
}





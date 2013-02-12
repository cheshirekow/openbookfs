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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Server.cpp
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "Server.h"
#include "ExceptionStream.h"
#include <iostream>
#include <fstream>
#include <string>

#include <crypto++/files.h>
#include <crypto++/cryptlib.h>


namespace   openbook {
namespace filesystem {


Server::Server()
{
    m_mutex.init();
}

Server::~Server()
{
    m_mutex.destroy();
}

void Server::initData(const std::string& dataDir)
{
    namespace fs = boost::filesystem;

    // check that the data directory and subdirectories exist
    if( !fs::exists( fs::path(dataDir) ) )
    {
        std::cout << "creating data directory: "
                  << fs::absolute( fs::path(dataDir) )
                  << std::endl;
        bool result = fs::create_directories( fs::path(dataDir ) );
        if( !result )
            ex()() << "failed to create data directory: " << dataDir;
    }

    if( !fs::exists( fs::path(dataDir) / "client_keys" ) )
    {
        std::cout << "creating directory : "
                  << fs::absolute( fs::path(dataDir) / "client_keys" )
                  << std::endl;
        bool result = fs::create_directories( fs::path(dataDir) / "client_keys" );
        if( !result )
            ex()() << "failed to create data directory: "
                      << dataDir + "/client_keys";
    }
}





} // namespace filesystem
} // namespace openbook

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
 *  @file   src/client_fs/Configuration.h
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_CONFIGURATION_H_
#define OPENBOOK_CONFIGURATION_H_



#include <bitset>

#include <cpp-pthreads.h>
#include <crypto++/rsa.h>
#include <cryptopp/osrng.h>
#include <boost/filesystem.hpp>

#include "KVStore.h"


namespace   openbook {
namespace filesystem {




/// encapsulates details about the client and it's configuration
struct Configuration:
    public KVStore<std::string,int>
{
    public:
        typedef KVStore<std::string,int>    KVStore_t;
        typedef boost::filesystem::path     path;

    private:
        pthreads::Mutex m_mutex;        ///< locks this data
        std::string     m_configFile;   ///< the configuration file

    public:
        Configuration();
        ~Configuration();

    private:
        /// called when the data directory changes
        void initializeDataDir();

    public:
        /// gui interface
        void setConfigFile();

        /// load a configuration from a file (will throw an exception on
        /// errors)
        void loadConfig();
        void loadConfig( const std::string& configFile );
        void saveConfig();
        void saveConfig( const std::string& configFile );

        path getPath( const std::string& key );

};



} // namespace filesystem
} // namespace openbook

#endif // CONFIGURATION_H_

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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/KVStore.h
 *
 *  @date   Apr 1, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_KVSTORE_H_
#define OPENBOOK_FS_KVSTORE_H_

#include <fstream>

#include <boost/filesystem.hpp>
#include <re2/re2.h>
#include <yaml-cpp/yaml.h>

#include "ExceptionStream.h"

namespace   openbook {
namespace filesystem {
namespace    kvstore {

/// base class for kv store, provides storage for one type
template <typename Data>
struct Storage
{
    typedef std::map<std::string,Data> Map_t;

    Map_t  map;

    void write( YAML::Emitter& out )
    {
        for( typename Map_t::iterator
                iPair = map.begin(); iPair != map.end(); ++iPair )
        {
            out << YAML::Key   << iPair->first
                << YAML::Value << iPair->second;
        }
    }

    void read( YAML::Node& in )
    {
        for( typename Map_t::iterator
                iPair = map.begin(); iPair != map.end(); ++iPair )
        {
            if( const YAML::Node *node = in.FindValue(iPair->first) )
            {
                (*node) >> iPair->second;
            }
        }
    }
};




/// variadic base class
template <typename... Datas>
struct Base;

/// specialization for base case
template <typename Data>
struct Base<Data>:
    public Storage<Data>
{
    void write( YAML::Emitter& out )
    {
        Storage<Data>::write(out);
    }

    void read( YAML::Node& in )
    {
        Storage<Data>::read(in);
    }
};

/// specialization for recursion
template <typename First, typename... Rest>
struct Base<First,Rest...>:
    public Base<First>,
    public Base<Rest...>
{
    void write( YAML::Emitter& out )
    {
        Base<First>::write(out);
        Base<Rest...>::write(out);
    }

    void read( YAML::Node& in )
    {
        Base<First>::read(in);
        Base<Rest...>::read(in);
    }
};


} // namespace kvstore

/// type safe key/value store
template <typename... Datas>
struct KVStore:
    public kvstore::Base<Datas...>
{
    template <typename T>
    T& put( const std::string& key )
    {
        typedef kvstore::Storage<T>  Storage_t;
        return Storage_t::map[key];
    }

    template <typename T>
    const T& get( const std::string& key ) const
    {
        typedef kvstore::Storage<T>  Storage_t;
        typename Storage_t::Map_t::const_iterator iPair =
                                                Storage_t::map.find(key);
        return iPair->second;
    }

    template <typename T>
    void set( const std::string& key, const T& value )
    {
        typedef kvstore::Storage<T>  Storage_t;
        Storage_t::map[key] = value;
    }

    /// initialize the kv-store with default values
    template <typename T>
    KVStore<Datas...>& operator()( const std::string& key, const T& value )
    {
        set(key,value);
        return *this;
    }

    void read( const std::string yamlFile )
    {
        std::ifstream in( yamlFile.c_str() );
        if( !in.good() )
            ex()() << "Failed to open " << yamlFile << " for reading";
        YAML::Parser parser(in);
        YAML::Node   doc;
        while( parser.GetNextDocument(doc) )
            kvstore::Base<Datas...>::read(doc);
        in.close();
    }

    void write( const std::string yamlFile )
    {
        std::ofstream out( yamlFile.c_str() );
        if( !out.good() )
            ex()() << "Failed to open " << yamlFile << " for writing";
        YAML::Emitter yaml;
        yaml << YAML::BeginMap;
        kvstore::Base<Datas...>::write(yaml);
        yaml << YAML::EndMap;
        out << yaml.c_str();
        out.close();
    }
};


} // namespace filesystem
} // namespace openbook



#endif // KVSTORE_H_

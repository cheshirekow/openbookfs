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
 *  @file   src/backend/Backend.cpp
 *
 *  @date   Apr 9, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <ctime>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <crypto++/files.h>
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <soci/soci.h>
#include <tclap/CmdLine.h>
#include <yaml-cpp/yaml.h>

#include "fuse_include.h"
#include "global.h"
#include "Backend.h"
#include "Connection.h"
#include "MessageHandler.h"
#include <crypto++/base64.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include "SelectSpec.h"
#include "MetaFile.h"







namespace   openbook {
namespace filesystem {

Backend::Backend()
{
    m_mutex.init();
    m_configFile  = "./obfs.yaml";
    m_displayName = "Anonymous";
    m_dataDir     = "./.obfs";

    m_maxPeers = 10;
    m_connPool.reserve(m_maxPeers);
    m_workerPool.reserve(m_maxPeers);

    for(int i=0;i < m_maxPeers; i++)
    {
        Connection* conn       = new Connection();
        MessageHandler* worker = new MessageHandler();
        conn->init(this,&m_connPool);
        worker->init(this,&m_workerPool);
    }

    // connect socket listeners
    m_listeners[LISTEN_LOCAL].sig_client.connect(
            sigc::bind<-1>( sigc::mem_fun(*this,&Backend::onConnect),false) );
    m_listeners[LISTEN_REMOTE].sig_client.connect(
            sigc::bind<-1>( sigc::mem_fun(*this,&Backend::onConnect),true) );

    m_clientFamily = AF_UNSPEC;
    m_clientNode   = "";
}

Backend::~Backend()
{
    m_mutex.destroy();
}

const std::string& Backend::displayName()
{
    return m_displayName;
}

const std::string& Backend::publicKey()
{
    return m_pubKey;
}

int Backend::registerPeer( const std::string& base64,
                           const std::string& displayName,
                           Connection* conn )
{
    // if public key is empty string then that means this is a GUI only
    // connection so we dont bother putting it in the map
    std::cout << "Putting base64 client key into db:\n"
              << base64 << std::endl;

    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    // insert the key into the database if it isn't already there
    sql << "INSERT OR IGNORE INTO known_clients (client_key, client_name) "
           "VALUES ('"<< base64 << "','" << displayName << "')";

    // now select out the id
    int peerId = 0;
    sql << "SELECT client_id FROM known_clients WHERE client_key='"
        << base64 << "'",
            soci::into(peerId);

    // update the client name
    sql << "UPDATE known_clients SET client_name='" << displayName
        << "' WHERE client_id=" << peerId;

    // update the local map if necessary
    typedef std::pair<std::string,int>  mapentry;
    m_idMap.lockFor()->insert( mapentry(base64,peerId) );

    // lock scope
    {
        // the map lock
        pthreads::ScopedLock lock( m_peerMap.mutex() );

        // now map the client id to this object
        // if another Connection is in the map for this client then we must
        // wait for it to remove itself before we put this in the map as the
        // handler for the client
        while( m_peerMap.subvert()->find(peerId)
                != m_peerMap.subvert()->end() )
        {
            std::cout << "Backend: can't insert connection into the map for "
                         "client " << peerId << ", waiting\n";

            // releases the lock, and then waits for someone else to aquire
            // and release it
            m_peerMap.wait();
        }

        std::cout << "Backend: putting connection " << (void*)conn
                  << " into the map for client " << peerId << "\n";
        (*( m_peerMap.subvert() ))[peerId] = conn;
        m_peerMap.signal();
    }
    return peerId;
}

void Backend::unregisterPeer( int peerId )
{
    std::cout << "Backend: removing " << peerId << "from map\n";
    m_peerMap.lockFor()->erase(peerId);
}


std::string Backend::privateKeyFile()
{
    return m_privKey.string();
}

void Backend::onConnect(FdPtr_t sockfd, bool remote)
{
    // note: pools are thread safe so theres no need to hold a lock here,
    // we can simply do the work

    std::cout << "Backend assigning a connection object and handler to new "
              << ( remote ? "remote" : "local" )
              << " peer on sockfd " << sockfd << "\n";
    Connection*     conn   = m_connPool.getAvailable();
    MessageHandler* worker = m_workerPool.getAvailable();
    if(!conn || !worker)
    {
        std::cerr << "Backend: No available connections, "
                     "closing peer socket\n";
        if(conn)
            m_connPool.reassign(conn);
        if(worker)
            m_workerPool.reassign(worker);
        return;
    }
    else
        conn->handleClient(remote,sockfd,worker);
}

void Backend::mount( const std::string& path,
                     const std::string& reldir,
                     int argc, char** argv )
{
    MountPoint* mp = new MountPoint(path);
    try
    {
        mp->mount(this,reldir,argc,argv);

        // lock access to m_mountPts
        m_mountPts.lockFor()->push_back(mp);
    }
    catch( const std::exception& ex )
    {
        delete mp;
        throw;
    }
}

void Backend::unmount( int imp )
{
    MountPoint* mp = 0;
    if( imp < 0 )
        ex()() << "Can't do anything with a negative index: " << imp;

    { // critical section for atomic update to m_mountPts
        LockedPtr<USMountMap_t> mountMap( &m_mountPts );

        if( imp >= mountMap->size() )
            ex()() << "Invalid mount point index: " << imp
                   << ", size: " << mountMap->size();

        mp = (*mountMap)[imp];
        if( imp != mountMap->size()-1 )
            (*mountMap)[imp] = mountMap->back();
        mountMap->pop_back();
    }

    // actually do the unmount, will block
    mp->unmount();

    // delete the structure
    delete mp;
}



void Backend::mapPeer( const messages::IdMapEntry& entry, std::map<int,int>& map )
{
    LockedPtr<USIdMap_t> idMap( &m_idMap );
    int peerId = 0;

    // if we do not know about this peer, then add him to the map
    USIdMap_t::iterator match = idMap->find( entry.publickey() );
    if( match == idMap->end() )
    {
        // create sqlite connection
        soci::session sql(soci::sqlite3, m_dbFile.string() );

        // insert the key into the database if it isn't already there
        sql << "INSERT OR IGNORE INTO known_clients (client_key, client_name) "
               "VALUES ('"
                    << entry.publickey()   << "','"
                    << entry.displayname()
            << "')";

        // now select out the id
        sql << "SELECT client_id FROM known_clients WHERE client_key='"
            << entry.publickey() << "'",
                soci::into(peerId);

        // add that peer to the map
        (*idMap)[entry.publickey()] = peerId;
    }
    else
        peerId = match->second;

    map[entry.peerid()] = peerId;
}

void Backend::setDisplayName( const std::string& name )
{
    pthreads::ScopedLock lock(m_mutex);

    std::cout << "Backend: setting display name: " << name << "\n";
    m_displayName = name;
}

void Backend::setDataDir( const std::string& dir )
{
    pthreads::ScopedLock lock(m_mutex);

    std::cout << "Backend: setting data dir: " << dir << "\n";

    m_dataDir = dir;
    m_rootDir = m_dataDir / "root";
    namespace fs = boost::filesystem;

    // check that the data directory and subdirectories exist
    if( !fs::exists( fs::path(m_dataDir) ) )
    {
        std::cout << "creating data directory: "
                  << fs::absolute( fs::path(m_dataDir) )
                  << std::endl;
        bool result = fs::create_directories( fs::path(m_dataDir ) );
        if( !result )
            ex()() << "failed to create data directory: " << m_dataDir;
    }

    // check that the root directory exists
    if( !fs::exists( fs::path(m_rootDir) ) )
    {
        std::cout << "creating root directory: "
                  << fs::absolute( fs::path(m_rootDir) )
                  << std::endl;
        bool result = fs::create_directories( fs::path(m_rootDir ) );
        if( !result )
            ex()() << "failed to create root directory: " << m_rootDir;
    }

    // if there is no private key file then create one
    m_privKey       = m_dataDir / "id_rsa.der";
    Path_t pubKey   = m_dataDir / "id_rsa_pub.der";

    if( !fs::exists( m_privKey ) || !fs::exists ( pubKey ) )
    {
        std::cout << "No public or private keyfile in "
                     "data directory, generating now\n";

        using namespace CryptoPP;
        AutoSeededRandomPool rng;
        RSA::PrivateKey rsaPrivate;
        rsaPrivate.GenerateRandomWithKeySize(rng, 3072 );
        RSA::PublicKey  rsaPublic(rsaPrivate);

        ByteQueue queue;

        // save private key
        rsaPrivate.Save(queue);
        FileSink privKeySink( m_privKey.string().c_str() );
        queue.CopyTo(privKeySink);
        privKeySink.MessageEnd();
        queue.Clear();

        // save public key
        rsaPublic.Save(queue);
        FileSink pubKeySink( pubKey.string().c_str() );
        queue.CopyTo( pubKeySink );
        pubKeySink.MessageEnd();
        queue.Clear();
    }

    // now base64 encode the public key and load into memory
    std::ifstream pubKeyIn( pubKey.string() );
    if( !pubKeyIn.good() )
    {
        ex()() << "Failed to open private key: "
               << pubKey
               << "for reading";
    }
    else
    {
        using namespace CryptoPP;
        FileSource pubKeyFile( pubKeyIn, true );
        ByteQueue queue;
        pubKeyFile.TransferTo(queue);
        queue.MessageEnd();
        RSA::PublicKey myKey;
        myKey.Load(queue);
        queue.Clear();
        myKey.Save(queue);
        queue.MessageEnd();
        Base64Encoder encoder;
        queue.CopyTo(encoder);
        encoder.MessageEnd();
        std::stringstream pubKeyStream;
        FileSink pubKeySink( pubKeyStream );
        encoder.CopyTo( pubKeySink );
        m_pubKey = pubKeyStream.str();
    }

    // initialize the message database
    using namespace soci;

    std::cout << "Initializing database" << std::endl;
    m_dbFile = fs::path(m_dataDir) / "store.sqlite";
    session sql(sqlite3,m_dbFile.string());

    sql << "CREATE TABLE IF NOT EXISTS conflict_files ("
            "conflict_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "path TEXT NOT NULL ) ";

    sql << "CREATE TABLE IF NOT EXISTS downloads ("
            "tx_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "path TEXT NOT NULL ) ";

    sql << "CREATE TABLE IF NOT EXISTS current_messages ("
            "msg_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "msg_type INTEGER NOT NULL, "
            "msg BLOB)";

    sql << "CREATE TABLE IF NOT EXISTS old_messages ("
            "msg_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "msg_type INTEGER NOT NULL, "
            "msg BLOB)";

    sql << "CREATE TABLE IF NOT EXISTS known_clients ("
            "client_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "client_key TEXT NOT NULL UNIQUE, "
            "client_name TEXT NOT NULL) ";

    // initialize the id map
    std::string base64Key;
    int         peerId;
    typedef soci::rowset<soci::row>    rowset;
    typedef std::pair<std::string,int> mapentry;
    rowset rs =
            ( sql.prepare << "select client_id,client_key FROM known_clients");
    for( auto& row : rs )
        m_idMap.lockFor()->insert(
                mapentry(row.get<std::string>(1) ,row.get<int>(0) ) );

    sql.close();

    // initialize the root directory if not already initialized
    MetaFile rootMeta( m_rootDir );
    rootMeta.init();

    std::cout << "Done initializing\n";
}

void Backend::setLocalSocket( int port )
{
    pthreads::ScopedLock lock(m_mutex);

    std::cout << "Backend: setting local socket to port: " << port << "\n";
    m_listeners[LISTEN_LOCAL].setInterface( "localhost", port);
}

void Backend::setRemoteSocket( int addressFamily,
                                const std::string& node,
                                const std::string& service )
{
    pthreads::ScopedLock lock(m_mutex);

    std::cout << "Backend: setting remote socket to : "
              << node << ":" << service << "\n";
    m_listeners[LISTEN_REMOTE].setInterface( addressFamily, node, service );
}

void Backend::setClientSocket( int addressFamily,
                                const std::string& node )
{
    pthreads::ScopedLock lock(m_mutex);

    std::cout << "Backend: setting client socket to : "
              << node << "\n";
    m_clientFamily = addressFamily;
    m_clientNode   = node;
}

void Backend::setMaxConnections( int maxConnections )
{
    pthreads::ScopedLock lock(m_mutex);

    std::cout << "Backend: max connections: " << maxConnections << "\n";
    m_maxPeers = maxConnections;
    int toCreate = m_maxPeers - m_connPool.capacity();
    m_connPool.reserve( m_maxPeers );
    m_workerPool.reserve(m_maxPeers );
    for(int i=0; i < toCreate; i++ )
    {
        Connection*     conn    = new Connection();
        MessageHandler* worker  = new MessageHandler();
        conn  ->init(this,&m_connPool);
        worker->init(this,&m_workerPool);
    }
}

void Backend::loadConfig( const std::string& filename )
{
    namespace fs = boost::filesystem;

    // verify that the config file exists
    if( !fs::exists( fs::path(filename) ) )
        ex()() << "Configuration file not found: " << filename;

    std::ifstream in(filename.c_str());
    if(!in)
        ex()() << "Failed to open " << filename << " for reading";

    YAML::Parser parser(in);
    YAML::Node   config;
    parser.GetNextDocument(config);

    // any errors will throw an exception
    const YAML::Node*  node;
    std::string  strVal,strVal2;
    int          intVal;

    node = config.FindValue("displayName");
    if(node)
    {
        std::cout << "Config: Reading display name\n";
        (*node) >> strVal;
        setDisplayName( strVal );
    }

    node = config.FindValue("dataDir");
    if(node)
    {
        std::cout << "Config: Reading data dir\n";
        (*node) >> strVal;
        setDataDir(strVal);
    }

    node = config.FindValue("localSocket");
    if(node)
    {
        std::cout << "Config: Reading local socket\n";
        const YAML::Node* node2 = node->FindValue("service");
        intVal = 3030;
        if(node2)
        {
            (*node2) >> strVal;
            std::cout << "Config: local socket: " << strVal << "\n";
            (*node2) >> intVal;
            std::cout << "Config: setting local socket to " << intVal << "\n";
            setLocalSocket( intVal );
        }
        else
            std::cerr << "Found localSocket entry but no service\n";
    }

    node = config.FindValue("remoteSocket");
    if(node)
    {
        std::cout << "Config: Reading remote socket\n";
        int addr_family;
        std::string addr_node,addr_service;
        const YAML::Node* node2;

        addr_family = AF_UNSPEC;
        node2 = node->FindValue("family");
        if(node2)
        {
            *node2 >> strVal;
            if( strVal.compare("AF_INET")==0 )
                addr_family = AF_INET;
            else if( strVal.compare("AF_INET6")==0 )
                addr_family = AF_INET6;
            else if( strVal.compare("AF_UNSPEC")==0 )
                addr_family = AF_UNSPEC;
            else
            {
                std::cerr << "Failed to determine address family " << strVal
                          << ", using AF_UNSPEC";
            }
        }

        node2 = node->FindValue("node");
        if(node2)
        {
            (*node2) >> addr_node;
            if(addr_node.compare("any")==0)
                addr_node = "";
        }

        node2 = node->FindValue("service");
        if(node2)
            (*node2) >> addr_service;

        setRemoteSocket( addr_family, addr_node, addr_service );
    }

    node = config.FindValue("clientSocket");
    if(node)
    {
        int addr_family;
        std::string addr_node;
        const YAML::Node* node2;

        addr_family = AF_UNSPEC;
        node2 = node->FindValue("family");
        if(node2)
        {
            *node2 >> strVal;
            if( strVal.compare("AF_INET")==0 )
                addr_family = AF_INET;
            else if( strVal.compare("AF_INET6")==0 )
                addr_family = AF_INET6;
            else if( strVal.compare("AF_UNSPEC")==0 )
                addr_family = AF_UNSPEC;
            else
            {
                std::cerr << "Failed to determine address family " << strVal
                          << ", using AF_UNSPEC";
            }
        }

        node2 = node->FindValue("node");
        if(node2)
        {
            (*node2) >> addr_node;
            if(addr_node.compare("any")==0)
                addr_node = "";
        }

        setClientSocket( addr_family, addr_node);
    }

    node = config.FindValue("maxConnections");
    if( node )
    {
        (*node) >> intVal;
        setMaxConnections( intVal );
    }

    node = config.FindValue("mountPoints");
    if( node )
    {
        for(int i=0; i < node->size(); i++)
        {
            const int   nargs =100;
            const int   nchars=256;

            std::string mountPoint;
            std::string relDir;
            char        argBuf[nchars]; //< buffer for arguments
            int         argw = 0;       //< write offset
            char*       argv[nargs];    //< argument index
            int         argc = 0;       //< number of arguments

            // zero out contents for ease of debugging, and to implicitly
            // set null terminals for strings
            memset(argBuf,0,sizeof(argBuf));
            memset(argv,0,sizeof(argv));

            const YAML::Node* node2;
            node2 = (*node)[i].FindValue("mount");
            if(node2)
                (*node2) >> mountPoint;
            else
            {
                std::cerr << "Backend::loadConfig: "
                          << "mount point " << i << " is missing 'mount'\n";
                continue;
            }
            node2 = (*node)[i].FindValue("relDir");
            if(node2)
            {
                (*node2) >> relDir;
                if( relDir == "/" || relDir == "~" )
                    relDir = "";
            }
            else
                relDir = "";


            node2 = (*node)[i].FindValue("argv");
            if(node2)
            {
                char*   pwrite = argBuf; //< write head

                // for each argument in the sequence
                for(int j=0; j < node2->size(); j++)
                {
                    // retrieve the argument into a string
                    std::string arg;
                    (*node2)[j] >> arg;

                    // number of chars left in buffer
                    int remainder = nchars - (pwrite - argBuf);

                    // if we dont have room for the argument then quit
                    if( arg.length() + 1 > remainder )
                        break;

                    argv[argc++] = pwrite;  //< point the j'th argument to
                                            //  current write head
                    arg.copy(pwrite,arg.length(),0);    //< copy the argument
                    pwrite += arg.length() + 1;   //< advance the write head
                }
            }

            try
            {
                std::cout << "Backend::loadConfig mounting:"
                          << "\n  mount point: " << mountPoint
                          << "\n relative dir: " << relDir
                          << "\n         args: ";
                for(int i=0; i < argc; i++)
                    std::cout << "\n        " << argv[i];
                std::cout << "\n";

                mount(mountPoint,relDir,argc,argv);
            }
            catch( const std::exception& ex )
            {
                std::cerr << "Backend::loadConfig: Failed to mount "
                          << mountPoint << "\n";
            }
        }
    }


}

void Backend::saveConfig( const std::string& filename )
{
    pthreads::ScopedLock lock(m_mutex);
    namespace fs = boost::filesystem;

    // verify that the config file exists
    std::ofstream out(filename.c_str());
    if(!out)
        ex()() << "Failed to open " << filename << " for writing";

    std::string clientFamily;
    switch(m_clientFamily)
    {
        case AF_INET:
            clientFamily = "AF_INET";
            break;
        case AF_INET6:
            clientFamily = "AF_INET6";
            break;
        default:
            clientFamily = "AF_UNSPEC";
            break;
    }

    std::string clientNode = m_clientNode;
    if( m_clientNode.size() < 1 )
        clientNode = "any";

    YAML::Emitter yaml;
    yaml << YAML::BeginMap
         << YAML::Key   << "displayName"
         << YAML::Value << m_displayName
         << YAML::Key   << "dataDir"
         << YAML::Value << m_dataDir.string()
         << YAML::Key   << "localSocket"
         << YAML::Value
             << YAML::BeginMap
             << YAML::Key   << "service"
             << YAML::Value << m_listeners[LISTEN_LOCAL].getService()
             << YAML::EndMap
         << YAML::Key   << "remoteSocket"
         << YAML::Value
             << YAML::BeginMap
             << YAML::Key   << "family"
             << YAML::Value << m_listeners[LISTEN_REMOTE].getFamily()
             << YAML::Key   << "node"
             << YAML::Value << m_listeners[LISTEN_REMOTE].getNode()
             << YAML::Key   << "service"
             << YAML::Value << m_listeners[LISTEN_REMOTE].getService()
             << YAML::EndMap
         << YAML::Key   << "clientSocket"
         << YAML::Value
             << YAML::BeginMap
             << YAML::Key   << "family"
             << YAML::Value << clientFamily
             << YAML::Key   << "node"
             << YAML::Value << clientNode
             << YAML::EndMap
         << YAML::Key   << "maxConnections"
         << YAML::Value << m_maxPeers
         << YAML::Key   << "mountPoints"
         << YAML::Value
             << YAML::BeginSeq;

    { // lock scope
        pthreads::ScopedLock( m_mountPts.mutex() );
        USMountMap_t& mountPts = *(m_mountPts.subvert());
        for(int i=0; i < mountPts.size(); i++)
        {
            yaml << YAML::BeginMap
                    << YAML::Key   << "mount"
                    << YAML::Value << mountPts[i]->mountPoint()
                    << YAML::Key   << "reldir"
                    << YAML::Value << mountPts[i]->relDir()
                    << YAML::Key   << "argv"
                    << YAML::Value
                        << YAML::BeginSeq;

            MountPoint::argv_t::const_iterator iarg;
            for( iarg = mountPts[i]->argv();
                    iarg != mountPts[i]->argv_end();
                    ++iarg)
            {
                yaml << *iarg;
            }

            yaml        << YAML::EndSeq
                 << YAML::EndMap;
        }
    }


    yaml     << YAML::EndSeq
        << YAML::EndMap;

    out << yaml.c_str();
}

void Backend::attemptConnection( bool isRemote,
                                 const std::string& node,
                                 const std::string& service )
{
    pthreads::ScopedLock lock(m_mutex);

    // defaults
    addrinfo  hints;
    addrinfo* found;
    memset(&hints,0,sizeof(addrinfo));
    hints.ai_family   = m_clientFamily;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    int sockfd = 0;
    if( m_clientNode.size() < 1 )
    {
        sockfd = socket( hints.ai_family,
                         hints.ai_socktype,
                         hints.ai_protocol );
        if (sockfd < 0)
            ex()() << "Failed to create a socket for 'any'";
    }
    else
    {
        const char* node    = m_clientNode.c_str();
        const char* service = 0;

        int result = getaddrinfo(node,service,&hints,&found);
        if( result < 0  )
        {
            ex()() << "Failed to find an interface which matches family: "
                   << m_clientFamily << ", node: "
                   << m_clientNode
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

            sockfd = socket(addr->ai_family,
                                addr->ai_socktype ,
                                addr->ai_protocol);
            if (sockfd < 0)
                continue;
            else
                break;
        }

        freeaddrinfo(found);

        if( !addr )
            ex()() << "None of the matched interfaces work";

        char host[NI_MAXHOST];
        char port[NI_MAXSERV];
        memset(host, 0, sizeof(host));
        memset(port, 0, sizeof(port));
        getnameinfo( (sockaddr*)addr->ai_addr, addr->ai_addrlen,
                     host, sizeof(host),
                     port, sizeof(port),
                     NI_NUMERICHOST | NI_NUMERICSERV );

        std::cout << "Using client interface " << host
                  << ":" << port << std::endl;
    }

    FdPtr_t clientfd = FileDescriptor::create(sockfd);

    // attempt to make connection
    const char* pnode    = node.c_str();
    const char* pservice = service.c_str();

    std::cout << "Searching for host addr matching "
              << node << ":" << service << std::endl;

    hints.ai_family = AF_UNSPEC;
    int result = getaddrinfo(pnode,pservice,&hints,&found);
    if( result < 0 )
    {
        ex()() << "Failed to find an interface which matches family: "
               << node << ":" << service
               << "\nErrno is " << errno << " : " << strerror(errno);
    }

    addrinfo* addr = found;

    for( ; addr; addr = addr->ai_next )
    {
        std::cout << "Attempting to connect to server:"
                  << "\n   family: " << addr->ai_family
                  << "\n     type: " << addr->ai_socktype
                  << "\n protocol: " << addr->ai_protocol
                  << std::endl;

        int connectResult =
                connect( *clientfd, addr->ai_addr, addr->ai_addrlen );
        if (connectResult < 0 )
        {
            std::cerr << "Connection failed, errno " << errno << " : "
                      << strerror(errno) << "\n";
            continue;
        }
        else
            break;
    }

    freeaddrinfo(found);

    if( !addr )
        ex()() << "None of the matched server interfaces work";

    // set the socket to nonblocking
    int flags = fcntl(*clientfd, F_GETFL, 0);
    fcntl(*clientfd, F_SETFL, flags | O_NONBLOCK);

    onConnect( clientfd, isRemote );
}

void Backend::getPeers( messages::PeerList* message )
{
    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    // now select out the id
    typedef soci::rowset<soci::row> rowset;
    rowset rs = ( sql.prepare << "SELECT client_id, client_name, client_key "
                           "FROM known_clients" );

    LockedPtr<USPeerMap_t> peerMap( &m_peerMap );

    for( auto& row : rs )
    {
        if( peerMap->find( row.get<int>(0) ) != peerMap->end() )
        {
            messages::PeerEntry* entry = message->add_peers();
            entry->set_peerid     ( row.get<int>(0)         );
            entry->set_displayname( row.get<std::string>(1) );
            entry->set_publickey  ( row.get<std::string>(2) );
        }
    }
}

void Backend::getKnownPeers( messages::PeerList* message )
{
    // create sqlite connection
    soci::session sql(soci::sqlite3, m_dbFile.string() );

    // now select out the id
    typedef soci::rowset<soci::row> rowset;
    rowset rs = ( sql.prepare << "SELECT client_id, client_name, client_key "
                           "FROM known_clients" );

    for( auto& row : rs )
    {
        messages::PeerEntry* entry = message->add_peers();
        entry->set_peerid     ( row.get<int>(0)         );
        entry->set_displayname( row.get<std::string>(1) );
        entry->set_publickey  ( row.get<std::string>(2) );
    }
}

void Backend::getMounts( messages::MountList* message )
{
    LockedPtr<USMountMap_t> mountMap( &m_mountPts );
    for( auto& mount : *mountMap )
    {
        messages::MountPoint* entry = message->add_mounts();
        entry->set_relpath( mount->relDir() );
        entry->set_path( mount->mountPoint() );
        for( auto& arg : mount->get_argv() )
        {
            std::string* arge = entry->add_argv();
            *arge = arg;
        }
    }
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
    { // lock scope
        pthreads::ScopedLock lock(m_mutex);

        parse(argc, argv);

        // for notifiying termination
        g_termNote = &m_termNote;

        // start the socket listeners
        for(int i=0; i < NUM_LISTENERS; i++)
            m_listenThreads[i].launch( SocketListener::start, m_listeners+i );

        // set their interfaces
        for(int i=0; i < NUM_LISTENERS; i++)
            m_listeners[i].setInterface( "localhost", 3030+i);

        // star the long job worker
        m_jobThread.launch( JobWorker::dispatch_main, &m_jobWorker );
    }

    sleep(1);
    // note: loadConfig locks so make sure this is outside previous lock
    // scope
    loadConfig( m_configFile );

    // wait for termination
    SelectSpec selectme;
    selectme.gen()
            ( g_termNote->readFd(), select_spec::READ );
    selectme.wait(false);

    // kill the job worker
    std::cout << "Backend: Killing job worker\n";
    m_jobWorker.enqueue( new JobKiller() );
    m_jobThread.join();

    // wait for listener threads to quit
    for(int i=0; i < NUM_LISTENERS; i++)
        m_listenThreads[i].join();

    // shut down fuse
    {
        pthreads::ScopedLock lock(m_mountPts.mutex());
        USMountMap_t& mountPts = *(m_mountPts.subvert());
        for(int i=0; i < mountPts.size(); i++)
            mountPts[i]->unmount();
    }

    // save the configuration
    // todo: there is a race condition right here, it's possible for someone
    // to modify the mountpoints while we're saving, perhaps change saveConfig
    // to have an option which enables/disables locking during the function
    saveConfig( m_configFile );

    // destroy mountpoints
    {
        pthreads::ScopedLock lock(m_mountPts.mutex());
        USMountMap_t& mountPts = *(m_mountPts.subvert());
        for(int i=0; i < mountPts.size(); i++)
            delete mountPts[i];
        mountPts.clear();
    }

    // wait for all connections to finish
    std::cout << "Backend: waiting for connections to finish\n";
    while(m_connPool.size() < m_connPool.capacity())
    {
        std::cout << m_connPool.capacity() - m_connPool.size()
                  << " to go\n";
        sleep(2);
    }


    return 0;
}

}
}

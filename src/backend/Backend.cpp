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







namespace   openbook {
namespace filesystem {

Backend::Backend()
{
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

}

const std::string& Backend::displayName()
{
    return m_displayName;
}

const std::string& Backend::publicKey()
{
    return m_pubKey;
}

int Backend::connectPeer( const std::string& publicKey )
{
    // if public key is empty string then that means this is a GUI only
    // connection so we dont bother putting it in the map
    if( publicKey.size() < 1 )
        return 0;
//    std::cout << "Putting base64 client key into db: " << base64 << std::endl;
//
//    // create sqlite connection
//    soci::session sql(soci::sqlite3,m_server->dbFile());
//
//    // insert the key into the database if it isn't already there
//    sql << "INSERT OR IGNORE INTO known_clients (client_key, client_name) "
//           "VALUES ('"<< base64 << "','" << displayName << "')";
//
//    // now select out the id
//    sql << "SELECT client_id FROM known_clients WHERE client_key='"
//        << base64 << "'",
//            soci::into(m_clientId);
//
//    // update the client name
//    sql << "UPDATE known_clients SET client_name='" << displayName
//        << "' WHERE client_id=" << m_clientId;

//    // lock scope
//    {
//        // wait for a lock on the map
//        pthreads::ScopedLock lock( m_clientMap->mutex() );
//
//        // now map the client id to this object
//        // if another Connection is in the map for this client then we must
//        // wait for it to remove itself before we put this in the map as the
//        // handler for the client
//        while( m_clientMap->subvert()->find(m_clientId)
//                != m_clientMap->subvert()->end() )
//        {
//            // releases the lock, and then waits for someone else to aquire
//            // and release it
//            m_clientMap->wait();
//        }
//
//        (*(m_clientMap->subvert()))[m_clientId] = this;
//        m_clientMap->signal();
//    }
    return 0;
}

void Backend::disconnectPeer( int peerId )
{
    if( peerId < 1 )
        return;
}

std::string Backend::privateKeyFile()
{
    return "***noKeyFileSet***";
}

void Backend::onConnect(FdPtr_t sockfd, bool remote)
{
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
        close(sockfd);
        return;
    }
    else
        conn->handleClient(remote,sockfd,worker);
}

void Backend::setDisplayName( const std::string& name )
{
    std::cout << "Backend: setting display name: " << name << "\n";
    m_displayName = name;
}

void Backend::setDataDir( const std::string& dir )
{
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
    sql.close();

    // initialize the root directory if not already initialized
    std::string rootMeta = (fs::path(m_rootDir) / "obfs.sqlite").string();
    sql.open(sqlite3,rootMeta);

    sql << "CREATE TABLE IF NOT EXISTS meta ("
            "key TEXT UNIQUE NOT NULL, "
            "value)";

    // note: states are
    // 0: synced
    // 1: dirty
    // 2: stale
    // 3: conflict
    sql << "INSERT OR IGNORE INTO meta (key,value) values ('state',0)";

    sql << "CREATE TABLE IF NOT EXISTS version ("
            "client TEXT UNIQUE NOT NULL, "
            "version INTEGER NOT NULL) ";

    // note: file types are
    // 1: regular file
    // 2: subdirectory
    // 3: symbolic link
    // 4: hard link (not yet supported)
    sql << "CREATE TABLE IF NOT EXISTS entries ("
            "path VARCHAR(255) UNIQUE NOT NULL, "
            "type INTEGER NOT NULL DEFAULT(1), "
            "subscribed INTEGER NOT NULL, "
            "size INTEGER NOT NULL, "
            "ctime INTEGER NOT NULL, "
            "mtime INTEGER NOT NULL) ";
    m_dataDir = dir;

    std::cout << "Done initializing\n";
}

void Backend::setLocalSocket( int port )
{
    std::cout << "Backend: setting local socket to port: " << port << "\n";
    m_listeners[LISTEN_LOCAL].setInterface( "localhost", port);
}

void Backend::setRemoteSocket( int addressFamily,
                                const std::string& node,
                                const std::string& service )
{
    std::cout << "Backend: setting remote socket to : "
              << node << ":" << service << "\n";
    m_listeners[LISTEN_REMOTE].setInterface( addressFamily, node, service );
}

void Backend::setClientSocket( int addressFamily,
                                const std::string& node )
{
    std::cout << "Backend: setting client socket to : "
              << node << "\n";
    m_clientFamily = addressFamily;
    m_clientNode   = node;
}

void Backend::setMaxConnections( int maxConnections )
{
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


}

void Backend::saveConfig( const std::string& filename )
{
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
         << YAML::EndMap;

    out << yaml.c_str();
}

void Backend::attemptConnection( const std::string& node,
                                 const std::string& service )
{
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
                         hints.ai_socktype ,
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

    onConnect( clientfd, true );
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

    for(int i=0; i < NUM_LISTENERS; i++)
        m_listeners[i].setInterface( "localhost", 3030+i);

    sleep(2);
    loadConfig( m_configFile );

    SelectSpec selectme;
    selectme.gen()( g_termNote->readFd(), select_spec::READ );
    selectme.wait();

    for(int i=0; i < NUM_LISTENERS; i++)
        m_listenThreads[i].join();

    saveConfig( m_configFile );
    return 0;
}

}
}

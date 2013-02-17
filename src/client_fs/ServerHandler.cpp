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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/ServerHandler.cpp
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */



#include <cerrno>
#include <sstream>
#include <csignal>
#include <fstream>

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/time.h>

#include <protobuf/message.h>
#include <protobuf/io/zero_copy_stream_impl.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>
#include <crypto++/files.h>
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/dh.h>
#include <crypto++/dh2.h>
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/cmac.h>
#include <crypto++/gcm.h>
#include <re2/re2.h>

#include "ServerHandler.h"
#include "SelectSpec.h"
#include "global.h"
#include "messages.h"
#include "messages.pb.h"
#include "jobs/QuitShouter.h"

namespace   openbook {
namespace filesystem {


void* ServerHandler::dispatch_main( void* vp_handler )
{
    ServerHandler* h = static_cast<ServerHandler*>(vp_handler);
    return h->main();
}

void* ServerHandler::dispatch_listen( void* vp_handler )
{
    ServerHandler* h = static_cast<ServerHandler*>(vp_handler);
    return h->listen();
}

void* ServerHandler::dispatch_shout( void* vp_handler )
{
    ServerHandler* h = static_cast<ServerHandler*>(vp_handler);
    return h->shout();
}




ServerHandler::ServerHandler()
{
    m_mutex.init();
}

ServerHandler::~ServerHandler()
{
    m_mutex.destroy();
}

void ServerHandler::init(
        Client* client, JobQueue_t* jobQueue, int termfd )
{
    using namespace pthreads;
    namespace cryp = CryptoPP;

    m_client    = client;
    m_newJobs   = jobQueue;
    m_fd[0]     = 0;
    m_fd[1]     = termfd;
    m_shouldDie = false;

    // load public and private keys
    try
    {
        // open a stream to the public key file
        std::ifstream in(client->pubKeyFile().c_str(),
                            std::ios::in | std::ios::binary);
        if (!in)
            ex()() << "Failed to open " << client->pubKeyFile().c_str()
                   << " for reading ";

        // seek to the end of the file to get it's size
        in.seekg(0, std::ios::end);

        // resize the storage space
        m_rsaPubStr.resize((unsigned int)in.tellg(),'\0');

        // seek back to the beginning
        in.seekg(0, std::ios::beg);

        // read in the entire file
        in.read(&m_rsaPubStr[0], m_rsaPubStr.size());

        // seek back to the beginning again
        in.seekg(0, std::ios::beg);

        // read into public key
        cryp::FileSource keyFile(in,true);
        cryp::ByteQueue  queue;
        keyFile.TransferTo(queue);
        queue.MessageEnd();

        m_rsaPubKey.Load(queue);
    }
    catch( cryp::Exception& cex )
    {
        ex()()  << "Failed to load public key from " << client->pubKeyFile()
                <<  " : " << cex.what() << std::endl;
    }

    try
    {
        cryp::FileSource keyFile(client->privKeyFile().c_str(),true);
        cryp::ByteQueue  queue;
        keyFile.TransferTo(queue);
        queue.MessageEnd();

        m_rsaPrivKey.Load(queue);
    }
    catch( cryp::Exception& cex )
    {
        ex()() << "Failed to load private key from " << client->privKeyFile()
                  <<  " : " << cex.what() << std::endl;
    }

    if( !m_rsaPubKey.Validate(m_rng,3) )
        ex()() << "Failed to validate public key" << std::endl;

    if( !m_rsaPrivKey.Validate(m_rng,3) )
        ex()() << "Failed to validate private key" << std::endl;
}


void ServerHandler::start()
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);

        int result = m_thread.launch(dispatch_main,this);
        if( result )
        {
            std::cerr << "Failed to start handler thread, errno " << result
                      << " : " << strerror(result) << std::endl;
        }
    }
}

void ServerHandler::join()
{
    m_thread.join();
}

//void ServerHandler::jobFinished(Job* job)
//{
//    // lock the handler so we know for a fact that the handler is in fact
//    // running or not
//    pthreads::ScopedLock(m_mutex);
//
//    m_finishedJobs.insert(job);
//}



void* ServerHandler::main()
{
    std::cout << "Starting handler " << (void*)this << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    while(!g_shouldDie)
    {
        std::cout << "Will start handler in 5 seconds" << std::endl;
        sleep(5);

        try
        {
            // lock scope
            {
                pthreads::ScopedLock(m_mutex);
                createConnection();
                handshake();
            }

            // set the connection to nonblocking
            int result = fcntl( m_fd[0], F_SETFL, O_NONBLOCK );
            if( result )
                ex()() << "Failed to set the socket to nonblocking";

            std::cout << "handler " << (void*)this
                      << " launching listen thread\n";
            m_listenThread.launch( dispatch_listen, this );

            std::cout << "handler " << (void*)this
                      << " launching shout thread\n";
            m_shoutThread.launch( dispatch_shout, this );

            m_listenThread.join();
            std::cout << "handler " << (void*)this
                      << " listen thread quit\n";

            m_shoutThread.join();
            std::cout << "handler " << (void*)this
                      << " shout thread quit\n";
        }
        catch( std::exception& ex )
        {
            std::cerr << "Exception in handler " << (void*)this << " main():\n   "
                      << ex.what() << std::endl;
        }

        // now we do our clean  up, but first lock the object so that no one tries
        // to modify us while we're doing this
        pthreads::ScopedLock(m_mutex);

        // if there is a sockfd then close if
        if( m_fd[0] > 0 )
            close(m_fd[0]);
    }


    // now it's time to terminate
//    // clear out the finished queue
//    Job* job=0;
//    while( !m_finishedJobs.empty() )
//    {
//        std::cout << "Handler " << (void*) this
//              << " removing unacked finished jobs\n";
//        m_finishedJobs.extract(job);
//        delete job;
//    }

    std::cout.flush();
    return 0;
}


void ServerHandler::createConnection()
{
    // parse the server string
    std::string hostName;
    std::string hostPort;
    RE2         serverRegex( "([^:]+):(\\d+)" );

    if( !serverRegex.ok() )
        ex()() << "Something is wrong with the regex: ([^:]+):(\\d+)";

    bool matchResult =
            RE2::FullMatch(m_client->server().c_str(),
                            serverRegex,&hostName,&hostPort);
    if( !matchResult )
        ex()() << "Failed to match [HOST]:[PORT] in string "
               << m_client->server();

    int& sockfd = m_fd[0];

    // defaults
    addrinfo  hints;
    addrinfo* found;
    memset(&hints,0,sizeof(addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    if( m_client->addressFamily() == "AF_INET" )
        hints.ai_family     = AF_INET;
    else if( m_client->addressFamily() == "AF_INET6" )
        hints.ai_family = AF_INET6;
    else if( m_client->addressFamily() != "AF_UNSPEC" )
    {
        std::cerr << "I dont understand the address family "
                  << m_client->addressFamily() << ", check the config file. "
                  << "I will use AF_UNSPEC to seach for an interface";
    }

    if( m_client->iface() == "any" )
    {
        sockfd = socket( hints.ai_family,
                         hints.ai_socktype ,
                         hints.ai_protocol );
        if (sockfd < 0)
            ex()() << "Failed to create a socket for 'any'";
    }
    else
    {
        const char* node    = m_client->iface().c_str();
        const char* service = 0;

        int result = getaddrinfo(node,service,&hints,&found);
        if( result < 0  )
        {
            ex()() << "Failed to find an interface which matches family: "
                   << m_client->addressFamily() << ", node: "
                   << m_client->iface()
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

        std::cout << "Using client interface to " << host
                  << ":" << port << std::endl;
    }

    // attempt to make connection
    const char* node    = hostName.c_str();
    const char* service = hostPort.c_str();

    std::cout << "Searching for host addr matching "
              << hostName << ":" << hostPort << std::endl;

    int result = getaddrinfo(node,service,&hints,&found);
    if( result < 0 )
    {
        ex()() << "Failed to find an interface which matches family: "
               << m_client->addressFamily() << ", node: "
               << hostName << ", server: " << hostPort
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
                connect( sockfd, addr->ai_addr, addr->ai_addrlen );
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
}



void ServerHandler::handshake()
{
    namespace cryp = CryptoPP;
    namespace msgs = messages;
    char type;
    int  sockfd = m_fd[0];

    // first we receive a DH parameter messages
    type = m_msg.read(sockfd);
    if( type != MSG_DH_PARAMS )
        ex()() << "Protocol error, first message received from server is "
               << messageIdToString(type) << "(" << (int)type << "), "
               << "expecting DH_PARAMS";

    msgs::DiffieHellmanParams* dhParams =
            static_cast<msgs::DiffieHellmanParams*>( m_msg[MSG_DH_PARAMS] );

    cryp::Integer p,q,g;
    p.Decode( (unsigned char*)&dhParams->p()[0],
                dhParams->p().size(),
                cryp::Integer::UNSIGNED );
    q.Decode( (unsigned char*)&dhParams->q()[0],
                dhParams->q().size(),
                cryp::Integer::UNSIGNED );
    g.Decode( (unsigned char*)&dhParams->g()[0],
                dhParams->g().size(),
                cryp::Integer::UNSIGNED );

    cryp::DH                dh;         //< Diffie-Hellman structure
    cryp::DH2               dh2(dh);    //< Diffie-Hellman structure
    cryp::SecByteBlock      spriv;      //< static private key
    cryp::SecByteBlock      spub;       //< static public key
    cryp::SecByteBlock      epriv;      //< ephemeral private key
    cryp::SecByteBlock      epub;       //< ephemeral public key
    cryp::SecByteBlock      shared;     //< shared key

    dh.AccessGroupParameters().Initialize(p,q,g);
    if( !dh.AccessGroupParameters().Validate(m_rng,3) )
        ex()() << "WOAH!! diffie-hellman parameters aren't valid... "
                  "possible tampering with packets";

    // initialize DH
    {
        std::cout << "Generating DH parameters, this may take a minute"
                  << std::endl;
        using namespace CryptoPP;
        spriv = SecByteBlock( dh2.StaticPrivateKeyLength() );
        spub  = SecByteBlock( dh2.StaticPublicKeyLength() );
        epriv = SecByteBlock( dh2.EphemeralPrivateKeyLength() );
        epub  = SecByteBlock( dh2.EphemeralPublicKeyLength() );

        dh2.GenerateStaticKeyPair(m_rng,spriv,spub);
        dh2.GenerateEphemeralKeyPair(m_rng,epriv, epub);
        shared= SecByteBlock( dh2.AgreedValueLength() );
        std::cout << "Finished generating DH parameters" << std::endl;
    }

    // the first message is a DH key exchange
    msgs::KeyExchange* keyEx =
            static_cast<msgs::KeyExchange*>( m_msg[MSG_KEY_EXCHANGE] );
    keyEx->set_skey(spub.BytePtr(),spub.SizeInBytes());
    keyEx->set_ekey(epub.BytePtr(),epub.SizeInBytes());

    // send unencrypted key exchange
    m_msg.write(sockfd,MSG_KEY_EXCHANGE);
    type = m_msg.read(sockfd);

    if( type != MSG_KEY_EXCHANGE )
        ex()()  << "Protocol error: expected KEY_EXCHANGE message, got "
                << messageIdToString(type);

    // read the servers keys
    cryp::SecByteBlock epubServer(
                    (unsigned char*)&keyEx->ekey()[0], keyEx->ekey().size() );
    cryp::SecByteBlock spubServer(
                    (unsigned char*)&keyEx->skey()[0], keyEx->skey().size() );

    // generate shared key
    if (!dh2.Agree(shared,spriv,epriv,spubServer,epubServer))
        ex()() << "Failed to agree on a shared key";

    // use shared secret to generate real keys
    // Take the leftmost 'n' bits for the KEK
    cryp::SecByteBlock kek(
            shared.BytePtr(), cryp::AES::DEFAULT_KEYLENGTH);

    // CMAC key follows the 'n' bits used for KEK
    cryp::SecByteBlock mack(
            &shared.BytePtr()[cryp::AES::DEFAULT_KEYLENGTH],
                                cryp::AES::BLOCKSIZE);
    cryp::CMAC<cryp::AES> cmac(mack.BytePtr(), mack.SizeInBytes());

    // extract and decode the content key
    // AES in ECB mode is fine - we're encrypting 1 block, so we don't need
    // padding
    cryp::ECB_Mode<cryp::AES>::Decryption aes;
    aes.SetKey(kek.BytePtr(), kek.SizeInBytes());

    // receive the content encryption key from the server
    type = m_msg.read(sockfd);
    if( type != MSG_CEK )
        ex()() << "Protocol error: expected CEK message, instead got "
               << messageIdToString(type) << "(" << (int)type << ") ";

    // verify message sizes
    msgs::ContentKey* contentKey =
            static_cast<msgs::ContentKey*>( m_msg[MSG_CEK] );
    if( contentKey->key().size() != cryp::AES::BLOCKSIZE )
        ex()() << "Message error: CEK key size "
               << contentKey->key().size() << " is incorrect, should be "
               << cryp::AES::BLOCKSIZE;

    if( contentKey->iv().size() != cryp::AES::BLOCKSIZE )
        ex()() << "Message error: CEK iv size "
               << contentKey->iv().size() << " is incorrect, should be "
               << cryp::AES::BLOCKSIZE;

    if( contentKey->cmac().size() != cryp::AES::BLOCKSIZE )
        ex()() << "Message error: CEK cmac size "
               << contentKey->cmac().size() << " is incorrect, should be "
               << cryp::AES::BLOCKSIZE;

    // Enc(CEK)|Enc(iv)
    cryp::SecByteBlock msg_cipher( 2*cryp::AES::BLOCKSIZE );
    memcpy( msg_cipher.BytePtr(), &contentKey->key()[0], cryp::AES::BLOCKSIZE );
    memcpy( &msg_cipher.BytePtr()[cryp::AES::BLOCKSIZE],
                                  &contentKey->iv()[0], cryp::AES::BLOCKSIZE );

    // Enc(CEK)
    cryp::SecByteBlock key_cipher(
                (unsigned char* )&contentKey->key()[0], cryp::AES::BLOCKSIZE );

    // ENC(IV)
    cryp::SecByteBlock iv_cipher (
                (unsigned char*)&contentKey->iv()[0], cryp::AES::BLOCKSIZE);

    // CMAC(Enc(CEK))
    cryp::SecByteBlock msg_cmac(
                (unsigned char* )&contentKey->cmac()[0], cryp::AES::BLOCKSIZE );

    // recompute cmac for verification
    cryp::SecByteBlock chk_cmac( cryp::AES::BLOCKSIZE );
    cmac.CalculateTruncatedDigest( chk_cmac.BytePtr(), cryp::AES::BLOCKSIZE,
                                   msg_cipher.BytePtr(), 2*cryp::AES::BLOCKSIZE );
    if( chk_cmac != msg_cmac )
        ex()() << "WOAH!!! CEK message digest doesn't match, possible "
                  "tampering of data";

    // decrypt the CEK
    m_cek = cryp::SecByteBlock( cryp::AES::DEFAULT_KEYLENGTH );
    m_iv  = cryp::SecByteBlock( cryp::AES::BLOCKSIZE );
    aes.ProcessData( m_cek.BytePtr(), key_cipher.BytePtr(), cryp::AES::DEFAULT_KEYLENGTH );
    aes.ProcessData(  m_iv.BytePtr(),  iv_cipher.BytePtr(), cryp::AES::BLOCKSIZE );

    cryp::Integer cekOut, ivOut;
    cekOut.Decode( m_cek.BytePtr(), m_cek.SizeInBytes() );
    ivOut .Decode(  m_iv.BytePtr(),  m_iv.SizeInBytes() );
    std::cout << "AES Key: " << std::hex << cekOut << std::endl;
    std::cout << "     iv: " << std::hex <<  ivOut << std::endl;
    std::cout << std::dec;

    // now we can make our encryptor and decryptor
    cryp::GCM<cryp::AES>::Encryption enc;
    cryp::GCM<cryp::AES>::Decryption dec;
    enc.SetKeyWithIV(m_cek.BytePtr(), m_cek.SizeInBytes(),
                     m_iv.BytePtr(), m_iv.SizeInBytes());
    dec.SetKeyWithIV(m_cek.BytePtr(), m_cek.SizeInBytes(),
                    m_iv.BytePtr(), m_iv.SizeInBytes());

    // the next message we send is the authentication message carrying our
    // public key
    msgs::AuthRequest* authReq =
            static_cast<msgs::AuthRequest*>(m_msg[MSG_AUTH_REQ]);
    authReq->set_public_key(m_rsaPubStr);

    m_msg.write(sockfd,MSG_AUTH_REQ,enc);
    enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

    // we expect a challenge
    type = m_msg.read(sockfd,dec);
    dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
    if( type != MSG_AUTH_CHALLENGE )
        ex()() << "Protocol error: expected AUTH_CHALLENGE but received "
               << messageIdToString(type) << " (" << (int)type << ")";

    std::string solution;
    msgs::AuthChallenge* authChallenge =
            static_cast<msgs::AuthChallenge*>(m_msg[MSG_AUTH_CHALLENGE]);

    if( authChallenge->type() != msgs::AuthChallenge::AUTHENTICATE )
        ex()() << "Protocol error: expected an authentication challenge";

    // create an RSA Decryptor to verify ownership
    cryp::RSAES_OAEP_SHA_Decryptor rsaDec( m_rsaPrivKey );
    solution.resize( rsaDec.FixedMaxPlaintextLength() );
    rsaDec.Decrypt(m_rng,
                    (unsigned char*)&authChallenge->challenge()[0],
                    authChallenge->challenge().size(),
                    (unsigned char*)&solution[0] );

    msgs::AuthSolution* authSoln =
            static_cast<msgs::AuthSolution*>(m_msg[MSG_AUTH_SOLN]);
    authSoln->set_solution(solution);

    m_msg.write(sockfd,MSG_AUTH_SOLN,enc);
    enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

    type = m_msg.read(sockfd,dec);
    dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
    if( type != MSG_AUTH_RESULT )
        ex()() << "Protocol Error: expected AUTH_RESULT but got "
               << messageIdToString(type) << " (" << (int)type << ")";

    msgs::AuthResult* authResult =
            static_cast<msgs::AuthResult*>( m_msg[MSG_AUTH_RESULT] );
    if( !authResult->response() )
        ex()() << "Failed the servers challenge";

    // now we expect a authorization challenge
    type = m_msg.read(sockfd,dec);
    dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

    if( type != MSG_AUTH_CHALLENGE )
        ex()() << "Protocol Error: expected AUTH_CHALLENGE but got "
               << messageIdToString(type) << " (" << (int)type << ")";
    if( authChallenge->type() != msgs::AuthChallenge::AUTHORIZE )
        ex()() << "Protocol Error: expected AUTHORIZE challenge";
    if( authChallenge->challenge().size() != cryp::SHA512::BLOCKSIZE )
        ex()() << "Expected salt of size " << cryp::SHA512::BLOCKSIZE
               << " but got one of " << authChallenge->challenge().size();

    cryp::SecByteBlock salt( cryp::SHA512::BLOCKSIZE );
    memcpy( salt.BytePtr(), authChallenge->challenge().c_str(),
                                            cryp::SHA512::BLOCKSIZE );

    std::string password = "fabulous";
    cryp::SHA512 hash;
    hash.Update( (unsigned char*)password.c_str(), password.size() );
    hash.Update( salt.BytePtr(), salt.SizeInBytes() );

    cryp::SecByteBlock digest( cryp::SHA512::DIGESTSIZE );
    hash.Final( digest.BytePtr() );
    authSoln->set_solution( digest.BytePtr(), digest.SizeInBytes() );

    std::cout << "Sending password challenge";
    cryp::Integer saltOut, hashOut;
    saltOut.Decode(salt.BytePtr(),salt.SizeInBytes() );
    hashOut.Decode(digest.BytePtr(),digest.SizeInBytes() );
    std::cout << "\n   salt: " << std::hex << saltOut
              << "\n   hash: " << hashOut << std::dec << std::endl;

    m_msg.write(sockfd,MSG_AUTH_SOLN,enc);
    enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes() );

    // read response
    type = m_msg.read(sockfd,dec);
    dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes() );

    if( type != MSG_AUTH_RESULT )
        ex()() << "Protocol Error: expected AUTH_RESULT but got "
               << messageIdToString(type) << " (" << (int)type << ")";

    if( authResult->response() )
        std::cout << "Authorized!!" << std::endl;
    else
        ex()() << "Not authorized";

}


void* ServerHandler::listen()
{
    using namespace CryptoPP;
    std::cout << "Starting client listener for handler" << (void*)this
              << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    try
    {
        GCM<AES>::Decryption dec;
        dec.SetKeyWithIV(m_cek.BytePtr(), m_cek.SizeInBytes(),
                         m_iv.BytePtr(),  m_iv.SizeInBytes());

        // infinite loop until exception is thrown
        while(1)
        {
            // read one message from server, exception thrown on disconnect
            // or termination signal
            char type = m_msg.read(m_fd,dec);
            dec.Resynchronize(m_iv.BytePtr(),m_iv.SizeInBytes());

            // generate a job from the message
            Job* job = 0;

            switch(type)
            {
                default:
                    std::cerr << "Handler " << (void*)this << " : "
                              << "Dont know what to do with a job request of "
                              << "message type (" << (int)type << ") : "
                              << messageIdToString(type) << std::endl;
                    break;
            }

            // put the job in the global queue, will block until queue has
            // capacity
             if(job)
                m_newJobs->insert(job);
        }
    }
    catch( std::exception& ex )
    {
        std::cout << "Handler " << (void*)this
                  << " listener is terminating on exception: "
                  << ex.what() << std::endl;
    }

    // put a dummy job into the queue so that the shouter can quit
    m_finishedJobs.insert( new jobs::QuitShouter(0,this) );

    return 0;
}

void* ServerHandler::shout()
{
    using namespace CryptoPP;
    std::cout << "Starting client shouter for handler" << (void*)this
              << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    try
    {
        GCM<AES>::Encryption enc;
        enc.SetKeyWithIV(m_cek.BytePtr(), m_cek.SizeInBytes(),
                          m_iv.BytePtr(),  m_iv.SizeInBytes());
        Job* job = 0;

        while(1)
        {
            // wait for a job to be finished from the queue
            m_finishedJobs.extract(job);

            // if the job is a "quit shouter" job then we just delete it
            // and break;
            if( job->derived() == jobs::QUIT_SHOUTER )
            {
                delete job;
                ex()() << "received QUIT_SHOUTER job";
            }

            // create a "job finished message", and fill it with the details
            messages::JobFinished* message =
                    static_cast<messages::JobFinished*>(m_msg[MSG_JOB_FINISHED]);
            message->set_job_id(job->id());

            // send the message
            m_msg.write(m_fd,MSG_JOB_FINISHED,enc);
            enc.Resynchronize(m_iv.BytePtr(),m_iv.SizeInBytes());
        }
    }
    catch( std::exception& ex )
    {
        std::cout << "Handler " << (void*)this
                  << " shouter is terminating on exception: "
                  << ex.what() << std::endl;
    }

    return 0;
}

void ServerHandler::jobFinished(Job* job)
{
    pthreads::ScopedLock lock(m_mutex);

     // first check to see if the client has died during the job
    if( job->version() == 0 )
        delete job;
    else
        m_finishedJobs.insert(job);
}




} // namespace filesystem
} // namespace openbook



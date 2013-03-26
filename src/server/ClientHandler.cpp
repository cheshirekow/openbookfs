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
 *  @file   src/server/ClientHandler.cpp
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <errno.h>
#include <sstream>
#include <csignal>

#include <protobuf/message.h>
#include <protobuf/io/zero_copy_stream_impl.h>

#include <crypto++/files.h>
#include <crypto++/cmac.h>
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/gcm.h>
#include <crypto++/base64.h>


#include "global.h"
#include "ClientHandler.h"
#include "SelectSpec.h"
#include "messages.h"
#include "messages.pb.h"

namespace   openbook {
namespace filesystem {
 namespace    server {


// calls ClientHandler::init
void* ClientHandler::dispatch_initDH( void* vp_handler )
{
    ClientHandler* h = static_cast<ClientHandler*>(vp_handler);
    return h->initDH();
}

void* ClientHandler::dispatch_main( void* vp_handler )
{
    ClientHandler* h = static_cast<ClientHandler*>(vp_handler);
    return h->main();
}

void* ClientHandler::dispatch_listen( void* vp_handler )
{
    ClientHandler* h = static_cast<ClientHandler*>(vp_handler);
    return h->listen();
}

void* ClientHandler::dispatch_shout( void* vp_handler )
{
    ClientHandler* h = static_cast<ClientHandler*>(vp_handler);
    return h->shout();
}




ClientHandler::ClientHandler():
    m_pool(0)
{
    m_mutex.init();
}

ClientHandler::~ClientHandler()
{
    m_mutex.destroy();
}

void ClientHandler::init(
        Pool_t* pool, Server* server, ClientMap* clientMap )
{
    using namespace pthreads;
    m_pool      = pool;
    m_server    = server;
    m_clientMap = clientMap;
    m_clientId  = 0;
    m_worker.init(server,&m_inboundMessages,&m_outboundMessages);

    std::cout << "Initializing handler " << (void*)this << std::endl;
    ScopedLock lock(m_mutex);

    // start DH parameter builder in thread
    Attr<Thread> attr;
    attr.init();
    attr << DETACHED;
    int result = m_thread.launch(attr,dispatch_initDH,this);
    attr.destroy();

    if( result )
    {
        std::cerr << "Failed to start handler thread, errno " << result
                  << " : " << strerror(result) << std::endl;
        m_pool->reassign(this);
    }
}

void* ClientHandler::initDH()
{
    pthreads::ScopedLock lock(m_mutex);
    std::cout << "Handler " << (void*) this
              << " generating DH parameters\n";

    CryptoPP::DH dh;
    dh.AccessGroupParameters().GenerateRandomWithKeySize(m_rng,1024);

    p = dh.GetGroupParameters().GetModulus();
    q = dh.GetGroupParameters().GetSubgroupOrder();
    g = dh.GetGroupParameters().GetGenerator();

    std::cout << "Handler " << (void*) this
              << " finished generating DH and returning to pool\n";
    m_pool->reassign(this);

    std::cout.flush();
    return 0;
}


void ClientHandler::handleClient( int sockfd, int termfd )
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);
        m_fd[0] = sockfd;
        m_fd[1] = termfd;

        Attr<Thread> attr;
        attr.init();
        attr << DETACHED;
        int result = m_thread.launch(attr,dispatch_main,this);
        attr.destroy();

        if( result )
        {
            std::cerr << "Failed to start handler thread, errno " << result
                      << " : " << strerror(result) << std::endl;
            m_pool->reassign(this);
        }
    }
}




void* ClientHandler::main()
{
    std::cout << "Starting handler " << (void*)this << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    // put our pointer into thread-local storage
    g_handlerKey.setSpecific(this);

    try
    {
        // lock scope
        {
            pthreads::ScopedLock(m_mutex);
            handshake();
            std::cout << "handler " << (void*)this
                      << " launching listen thread\n";
            m_listenThread.launch( dispatch_listen, this );

            std::cout << "handler " << (void*)this
                      << " launching shout thread\n";
            m_shoutThread.launch( dispatch_shout, this );

            // start the worker
            std::cout << "handler " << (void*)this
                      << " launching worker thread\n";
            m_worker.setClientId(m_clientId);
            m_workerThread.launch( MessageHandler::dispatch_main, &m_worker );

            // create a ping message for the client
            messages::Ping* ping = new messages::Ping();
            ping->set_payload(0xdeadf00d);
            TypedMessage pingMsg(MSG_PING,ping);
            m_outboundMessages.insert(pingMsg);

            sleep(2);
            messages::Pong* pong = new messages::Pong();
            pong->set_payload(0xdeadf00d);
            TypedMessage pongMsg(MSG_PONG,pong);
            m_outboundMessages.insert(pongMsg);
        }
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception in handler " << (void*)this << " main():\n   "
                  << ex.what();
    }

    m_listenThread.join();
    std::cout << "handler " << (void*)this << " listen thread quit\n";

    // put a quet message into the outgoing queue so that the shouter
    // knows its time to quit
    TypedMessage quit(MSG_QUIT);
    m_outboundMessages.insert( quit );

    m_shoutThread.join();
    std::cout << "handler " << (void*)this << " shout thread quit\n";

    // put a quit message into the inbound queue so that the worker knows
    // it's time to quit
    m_inboundMessages.insert( quit );

    m_workerThread.join();
    std::cout << "handler " << (void*)this << " worker thread quit\n";

    // remove ourselves from the client map
    m_clientMap->lockFor()->erase(m_clientId);

    // now we do our cleanup, but first lock the object so that no one tries
    // to modify us while we're doing this
    pthreads::ScopedLock lock(m_mutex);

    // invalidate client id so any jobs that finish after we die
    // don't get sent when this handler is reused later
    m_clientId=0;

    // close the file descriptor
    close(m_fd[0]);

    // generate new paramters for the next time we're run
    std::cout << "Handler " << (void*) this
              << " re-generating DH parameters\n";

    CryptoPP::DH dh;
    dh.AccessGroupParameters().GenerateRandomWithKeySize(m_rng,1024);

    p = dh.GetGroupParameters().GetModulus();
    q = dh.GetGroupParameters().GetSubgroupOrder();
    g = dh.GetGroupParameters().GetGenerator();

    std::cout << "Handler " << (void*) this
              << " finished re-generating DH and returning to pool\n";

    // clear out the finished queue
    TypedMessage msg;
    while( !m_outboundMessages.empty() )
    {
        std::cout << "Handler " << (void*) this
              << " removing unsent messages\n";
        m_outboundMessages.extract(msg);
        delete msg.msg;
    }



    // put ourselves back int the available pool
    m_pool->reassign(this);
    std::cout.flush();
    return 0;
}


void ClientHandler::handshake()
{
    namespace msgs = messages;
    using namespace pthreads;

    int received = 0;

    //create message buffers
    std::cerr << "handler " << (void*) this << " is starting up"
              << std::endl;

    using namespace CryptoPP;

    DH                dh;       //< Diffie-Hellman structure
    DH2               dh2(dh);  //< Diffie-Hellman structure

    dh.AccessGroupParameters().Initialize(p,q,g);
    SecByteBlock spriv( dh2.StaticPrivateKeyLength() );
    SecByteBlock spub ( dh2.StaticPublicKeyLength() );
    SecByteBlock epriv( dh2.EphemeralPrivateKeyLength() );
    SecByteBlock epub ( dh2.EphemeralPublicKeyLength() );

    dh2.GenerateStaticKeyPair(m_rng,spriv,spub);
    dh2.GenerateEphemeralKeyPair(m_rng,epriv, epub);
    SecByteBlock shared( dh2.AgreedValueLength() );

    // first the server sends the diffie hellman parameters so we can do a
    // key exchange
    std::string pStr,qStr,gStr;
    pStr.resize( p.ByteCount());
    qStr.resize( q.ByteCount());
    gStr.resize( g.ByteCount());

    p.Encode( (unsigned char*)&pStr[0], pStr.size(), Integer::UNSIGNED );
    q.Encode( (unsigned char*)&qStr[0], qStr.size(), Integer::UNSIGNED );
    g.Encode( (unsigned char*)&gStr[0], gStr.size(), Integer::UNSIGNED );

    msgs::DiffieHellmanParams* dhParams =
            static_cast<msgs::DiffieHellmanParams*>(m_msg[MSG_DH_PARAMS]);
    dhParams->set_p(pStr);
    dhParams->set_q(qStr);
    dhParams->set_g(gStr);

    std::cout << "Sending DH_PARAMS message " << std::dec << std::endl;
    m_msg.write(m_fd, MSG_DH_PARAMS);

    // the client must respond with a key exchange
    std::cout << "Waiting for KEY_EXCHANGE message " << std::endl;
    char type = m_msg.read(m_fd);
    if( type != MSG_KEY_EXCHANGE )
        ex()() << "Received message : " << messageIdToString(type)
               << " (" << (int)type << ") "
               << "when expecting MSG_KEY_EXCHANGE, terminating client";

    msgs::KeyExchange* keyEx =
            static_cast<msgs::KeyExchange*>(m_msg[MSG_KEY_EXCHANGE]);

    // read client keys
    SecByteBlock epubClient(
                    (unsigned char*)&keyEx->ekey()[0], keyEx->ekey().size() );
    SecByteBlock spubClient(
                    (unsigned char*)&keyEx->skey()[0], keyEx->skey().size() );

    // write out our keys
    keyEx->set_ekey( epub.BytePtr(), epub.SizeInBytes() );
    keyEx->set_skey( spub.BytePtr(), spub.SizeInBytes() );
    std::cout << "Sending KEY_EXCHANGE message " << std::endl;
    m_msg.write( m_fd, MSG_KEY_EXCHANGE );

    // generate shared key
    if( !dh2.Agree(shared,spriv,epriv,spubClient,epubClient) )
        ex()() << "Failed to agree on a shared key";

    Integer sharedOut;
    sharedOut.Decode(shared.BytePtr(),shared.SizeInBytes() );
    std::cout << "Shared secret (client): "
              << "\n   shared: " << std::hex << sharedOut
              << std::dec << std::endl;

    // use shared secret to generate real keys
    // Take the leftmost 'n' bits for the KEK
    SecByteBlock kek(
            shared.BytePtr(), AES::DEFAULT_KEYLENGTH);

    // CMAC key follows the 'n' bits used for KEK
    SecByteBlock mack(
            &shared.BytePtr()[AES::DEFAULT_KEYLENGTH],
                                AES::BLOCKSIZE);
    CMAC<AES> cmac(mack.BytePtr(), mack.SizeInBytes());

    // Generate a random CEK and IV
    m_cek = SecByteBlock(AES::DEFAULT_KEYLENGTH);
    m_iv  = SecByteBlock(AES::BLOCKSIZE);
    m_rng.GenerateBlock(m_cek.BytePtr(), m_cek.SizeInBytes());
    m_rng.GenerateBlock( m_iv.BytePtr(), m_iv.SizeInBytes());

    // initialize message buffer encoder, decoder
    m_msg.initAES(m_cek,m_iv);

    // print it for checkin
    Integer cekOut, ivOut;
    cekOut.Decode( m_cek.BytePtr(), m_cek.SizeInBytes() );
    ivOut .Decode(  m_iv.BytePtr(),  m_iv.SizeInBytes() );
    std::cout << "AES Key: " << std::hex << cekOut << std::endl;
    std::cout << "     iv: " << std::hex << ivOut  << std::endl;
    std::cout << std::dec;

    // AES in ECB mode is fine - we're encrypting 1 block, so we don't need
    // padding
    ECB_Mode<AES>::Encryption aes;
    aes.SetKey(kek.BytePtr(), kek.SizeInBytes());

    SecByteBlock msg_cipher( 2*AES::BLOCKSIZE );  //< Enc(CEK) | Enc(iv)
    SecByteBlock   msg_cmac(   AES::BLOCKSIZE );  //< CMAC(Enc(CEK||iv))

    aes.ProcessData(msg_cipher.BytePtr(), m_cek.BytePtr(), AES::BLOCKSIZE );
    aes.ProcessData(&msg_cipher.BytePtr()[AES::BLOCKSIZE],
                                          m_iv.BytePtr(), AES::BLOCKSIZE );
    cmac.CalculateTruncatedDigest(msg_cmac.BytePtr(), AES::BLOCKSIZE,
                                  msg_cipher.BytePtr(), 2*AES::BLOCKSIZE );

    // fill the content key message
    msgs::ContentKey* contentKey =
            static_cast<msgs::ContentKey*>( m_msg[MSG_CEK] );

    contentKey->set_key( msg_cipher.BytePtr(), AES::BLOCKSIZE );
    contentKey->set_iv(  &msg_cipher.BytePtr()[ AES::BLOCKSIZE],
                                               AES::BLOCKSIZE );
    contentKey->set_cmac( msg_cmac.BytePtr(), AES::BLOCKSIZE );

    // send the content key message
    std::cout << "Sending CEK message " << std::endl;
    m_msg.write(m_fd,MSG_CEK);

    // now we can make our encryptor and decriptor
    GCM<AES>::Encryption enc;
    GCM<AES>::Decryption dec;
    enc.SetKeyWithIV(m_cek.BytePtr(), m_cek.SizeInBytes(),
                      m_iv.BytePtr(),  m_iv.SizeInBytes());
    dec.SetKeyWithIV(m_cek.BytePtr(), m_cek.SizeInBytes(),
                      m_iv.BytePtr(),  m_iv.SizeInBytes());

    // read the client's public key
    std::cout << "Reading AUTH_REQ" << std::endl;
    type = m_msg.read(m_fd,dec);
    dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

    if( type != MSG_AUTH_REQ )
        ex()() << "Protocol Error: expected AUTH_REQ from client, instead got"
               << messageIdToString(type) << "(" << (int)type << ")";

    msgs::AuthRequest* authReq =
            static_cast<msgs::AuthRequest*>( m_msg[MSG_AUTH_REQ] );
    std::string displayName = authReq->display_name();
    std::stringstream  inkey( authReq->public_key() );
    FileSource keyFile(inkey,true);
    ByteQueue  queue;
    keyFile.TransferTo(queue);
    queue.MessageEnd();
    RSA::PublicKey clientKey;
    clientKey.Load(queue);

    // create an RSA Encryptor to verify ownership
    RSAES_OAEP_SHA_Encryptor rsaEnc( clientKey );

    // Now that there is a concrete object, we can validate
    if( rsaEnc.FixedMaxPlaintextLength() == 0 )
        ex()() << "Error generating RSA encryptor";

    // now we authenticate the client by sending a challenge of his key
    // ownership
    // generate a random string
    std::string plainChallenge;
    std::string cipherChallenge;
    plainChallenge.resize(rsaEnc.FixedMaxPlaintextLength());
    cipherChallenge.resize( rsaEnc.CiphertextLength(plainChallenge.size()) );

    m_rng.GenerateBlock(
            (unsigned char*)&(plainChallenge[0]), plainChallenge.size());
    rsaEnc.Encrypt( m_rng,
                    (unsigned char*)&plainChallenge[0],
                    plainChallenge.size(),
                    (unsigned char*)&cipherChallenge[0] );

    msgs::AuthChallenge* challenge =
            static_cast<msgs::AuthChallenge*>( m_msg[MSG_AUTH_CHALLENGE] );
    challenge->set_type( msgs::AuthChallenge::AUTHENTICATE );
    challenge->set_challenge(cipherChallenge);

    std::cout << "Sending AUTH_CHALLENGE" << std::endl;
    m_msg.write(m_fd,MSG_AUTH_CHALLENGE,enc);
    enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

    // read the challenge solution
    std::cout << "Waiting for AUTH_SOLUTION" << std::endl;
    type = m_msg.read(m_fd,dec);
    dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
    if( type != MSG_AUTH_SOLN )
        ex()() << "Protocol error: Expected AUTH_SOLN, got "
               << messageIdToString(type) << " (" << (int)type << ")";

    msgs::AuthSolution* authSoln =
            static_cast<msgs::AuthSolution*>( m_msg[MSG_AUTH_SOLN]);
    msgs::AuthResult* authResult =
            static_cast<msgs::AuthResult*>( m_msg[MSG_AUTH_RESULT] );
    if( authSoln->solution().compare(plainChallenge) != 0 )
    {
        authResult->set_response(false);
        m_msg.write(m_fd,MSG_AUTH_RESULT,enc);
        enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
        ex()() << "Client does not own the key";
    }

    authResult->set_response(true);
    m_msg.write(m_fd,MSG_AUTH_RESULT,enc);
    enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
    std::cout << "Client authenticated" << std::endl;

    // now that the client is authenticated we send an authorization challenge
    // start by creating a salt
    SecByteBlock salt( SHA512::BLOCKSIZE );
    m_rng.GenerateBlock( salt.BytePtr(), salt.SizeInBytes() );

    // now create the hashed password so we can verify the user's input
    SHA512 hash;
    hash.Update( (unsigned char*) m_server->password().c_str(),
                  m_server->password().size() );
    hash.Update( salt.BytePtr(), salt.SizeInBytes() );
    SecByteBlock digest( SHA512::DIGESTSIZE );
    hash.Final( digest.BytePtr() );

    std::cout << "Sending password challenge" << std::endl;
    Integer saltOut, hashOut;
    saltOut.Decode(salt.BytePtr(),salt.SizeInBytes() );
    hashOut.Decode(digest.BytePtr(),digest.SizeInBytes() );
    std::cout << "\n   pass: " << m_server->password()
              << "\n   salt: " << std::hex << saltOut
              << "\n   hash: " << hashOut << std::dec << std::endl;

    challenge->set_type( msgs::AuthChallenge::AUTHORIZE );
    challenge->set_challenge( salt.BytePtr(), salt.SizeInBytes() );

    // give the user some retries
    int authRetry = 0;
    for(; authRetry < 3; authRetry++)
    {
        // now send the challenge to the user
        m_msg.write(m_fd,MSG_AUTH_CHALLENGE,enc);
        enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

        // read the users reply
        type = m_msg.read(m_fd,dec);
        dec.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());

        if( type != MSG_AUTH_SOLN )
            ex()() << "Protocol error, expected AUTH_SOLN but got "
                   << messageIdToString(type) << " (" <<(int)type << ")";

        // if wrongs size
        if( authSoln->solution().size() != SHA512::DIGESTSIZE )
        {
            std::cout << "Wrong size digest" << std::endl;
            authResult->set_response(false);
            m_msg.write(m_fd,MSG_AUTH_RESULT,enc);
            enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
            continue;
        }

        // copy out the users response
        SecByteBlock soln( SHA512::DIGESTSIZE );
        memcpy( soln.BytePtr(), authSoln->solution().c_str(),
                                                SHA512::DIGESTSIZE );

        // if mismatch
        if( soln != digest )
        {
            std::cout << "Digest doesn't match" << std::endl;
            authResult->set_response(false);
            m_msg.write(m_fd,MSG_AUTH_RESULT,enc);
            enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
            continue;
        }

        // otherwise success
        authResult->set_response(true);
        m_msg.write(m_fd,MSG_AUTH_RESULT,enc);
        enc.Resynchronize(m_iv.BytePtr(), m_iv.SizeInBytes());
        break;
    }

    // if too many retries
    if( authRetry >= 3 )
        ex()() << "Client failed too many retries";

    // otherwise, authed
    std::cout << "Client is authorized" << std::endl;

    // base64 encode the clients public key
    queue.Clear();
    CryptoPP::Base64Encoder encoder;
    clientKey.Save(queue);
    queue.CopyTo(encoder);
    encoder.MessageEnd();
    std::string base64;
    base64.resize( encoder.MaxRetrievable(), '\0' );
    encoder.Get( (byte*)&base64[0], base64.size() );

    std::cout << "Putting base64 client key into db: " << base64 << std::endl;

    // create sqlite connection
    soci::session sql(soci::sqlite3,m_server->dbFile());

    // insert the key into the database if it isn't already there
    sql << "INSERT OR IGNORE INTO known_clients (client_key, client_name) "
           "VALUES ('"<< base64 << "','" << displayName << "')";

    // now select out the id
    sql << "SELECT client_id FROM known_clients WHERE client_key='"
        << base64 << "'",
            soci::into(m_clientId);

    // update the client name
    sql << "UPDATE known_clients SET client_name='" << displayName
        << "' WHERE client_id=" << m_clientId;


    // lock scope
    {
        // wait for a lock on the map
        pthreads::ScopedLock lock( m_clientMap->mutex() );

        // now map the client id to this object
        // if another ClientHandler is in the map for this client then we must
        // wait for it to remove itself before we put this in the map as the
        // handler for the client
        while( m_clientMap->subvert()->find(m_clientId)
                != m_clientMap->subvert()->end() )
        {
            // releases the lock, and then waits for someone else to aquire
            // and release it
            m_clientMap->wait();
        }

        (*(m_clientMap->subvert()))[m_clientId] = this;
        m_clientMap->signal();
    }
}

void* ClientHandler::listen()
{
    using namespace CryptoPP;
    std::cout << "Starting client listener for handler" << (void*)this
              << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    try
    {
        // infinite loop until exception is thrown
        while(1)
        {
            // read one message from client, exception thrown on disconnect
            // or termination signal
            TypedMessage msg = m_msg.readEnc(m_fd);

            // put the message in the inbound queue
            m_inboundMessages.insert(msg);
        }
    }
    catch( std::exception& ex )
    {
        std::cout << "Handler " << (void*)this
                  << " listener is terminating on exception: "
                  << ex.what() << std::endl;
    }

    return 0;
}

void* ClientHandler::shout()
{
    using namespace CryptoPP;
    std::cout << "Starting client shouter for handler" << (void*)this
              << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    try
    {
        while(1)
        {
            TypedMessage msg;

            // wait for a job to be finished from the queue
            m_outboundMessages.extract(msg);

            // if it's a quit message then quit
            if( msg.type == MSG_QUIT )
            {
                std::cout << "Client Handler " << (void*)this << " received a "
                             "QUIT_SHOUTER job, so quitting\n";
                break;
            }

            // otherwise send the message
            m_msg.writeEnc(m_fd,msg);
            delete msg.msg;
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






} // namespace server 
} // namespace filesystem
} // namespace openbook

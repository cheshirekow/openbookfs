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
 *  @file   src/server/Connection.cpp
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
#include "Connection.h"
#include "SelectSpec.h"
#include "messages.h"
#include "messages.pb.h"

namespace   openbook {
namespace filesystem {

static void validate_message( TypedMessage msg, MessageId expected )
{
    if( msg.type != expected )
    {
        ex()() << "Protocol Error: expected "
               << messageIdToString(expected)
               << " from peer, instead got"
               << messageIdToString(msg.type)
               << "(" << (int)msg.type << ")";
    }
}

Connection::Connection():
    m_pool(0)
{
    m_mutex.init();
}

Connection::~Connection()
{
    m_mutex.destroy();
}


void Connection::init( Pool_t* pool )
{
    using namespace pthreads;
    m_pool      = pool;
    m_peerId    = 0;

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

void Connection::handleClient( int sockfd )
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);
        m_sockfd = sockfd;

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





// calls Connection::init
void* Connection::dispatch_initDH( void* vp_handler )
{
    Connection* h = static_cast<Connection*>(vp_handler);
    return h->initDH();
}

void* Connection::dispatch_main( void* vp_handler )
{
    Connection* h = static_cast<Connection*>(vp_handler);
    return h->main();
}

void* Connection::dispatch_listen( void* vp_handler )
{
    Connection* h = static_cast<Connection*>(vp_handler);
    return h->listen();
}

void* Connection::dispatch_shout( void* vp_handler )
{
    Connection* h = static_cast<Connection*>(vp_handler);
    return h->shout();
}






void* Connection::initDH()
{
    pthreads::ScopedLock lock(m_mutex);
    std::cout << "Handler " << (void*) this
              << " generating DH parameters\n";

//    CryptoPP::DH dh;
//    dh.AccessGroupParameters().GenerateRandomWithKeySize(m_rng,1024);
//
//    p = dh.GetGroupParameters().GetModulus();
//    q = dh.GetGroupParameters().GetSubgroupOrder();
//    g = dh.GetGroupParameters().GetGenerator();

    using namespace CryptoPP;
    /// RFC 5114 2048-bit MODP Group with 256-bit Prime Order Subgroup
    p = Integer("0x"
            "87A8E61DB4B6663CFFBBD19C651959998CEEF608660DD0F2"
            "5D2CEED4435E3B00E00DF8F1D61957D4FAF7DF4561B2AA30"
            "16C3D91134096FAA3BF4296D830E9A7C209E0C6497517ABD"
            "5A8A9D306BCF67ED91F9E6725B4758C022E0B1EF4275BF7B"
            "6C5BFC11D45F9088B941F54EB1E59BB8BC39A0BF12307F5C"
            "4FDB70C581B23F76B63ACAE1CAA6B7902D52526735488A0E"
            "F13C6D9A51BFA4AB3AD8347796524D8EF6A167B5A41825D9"
            "67E144E5140564251CCACB83E6B486F6B3CA3F7971506026"
            "C0B857F689962856DED4010ABD0BE621C3A3960A54E710C3"
            "75F26375D7014103A4B54330C198AF126116D2276E11715F"
            "693877FAD7EF09CADB094AE91E1A1597");
    q = Integer("0x"
            "3FB32C9B73134D0B2E77506660EDBD484CA7B18F21EF2054"
            "07F4793A1A0BA12510DBC15077BE463FFF4FED4AAC0BB555"
            "BE3A6C1B0C6B47B1BC3773BF7E8C6F62901228F8C28CBB18"
            "A55AE31341000A650196F931C77A57F2DDF463E5E9EC144B"
            "777DE62AAAB8A8628AC376D282D6ED3864E67982428EBC83"
            "1D14348F6F2F9193B5045AF2767164E1DFC967C1FB3F2E55"
            "A4BD1BFFE83B9C80D052B985D182EA0ADB2A3B7313D3FE14"
            "C8484B1E052588B9B7D2BBD2DF016199ECD06E1557CD0915"
            "B3353BBB64E0EC377FD028370DF92B52C7891428CDC67EB6"
            "184B523D1DB246C32F63078490F00EF8D647D148D4795451"
            "5E2327CFEF98C582664B4C0F6CC41659");
    g = Integer("0x"
            "8CF83642A709A097B447997640129DA299B1A47D1EB3750B"
            "A308B0FE64F5FBD3");

    std::cout << "Handler " << (void*) this
              << " finished generating DH and returning to pool\n";
    m_pool->reassign(this);

    std::cout.flush();
    return 0;
}






void* Connection::main()
{
    std::cout << "Starting handler " << (void*)this << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    // put our pointer into thread-local storage
    // g_handlerKey.setSpecific(this);

    try
    {
        // lock scope
        pthreads::ScopedLock(m_mutex);
        handshake();
        std::cout << "handler " << (void*)this
                  << " launching listen thread\n";
        m_readThread.launch( dispatch_listen, this );

        std::cout << "handler " << (void*)this
                  << " launching shout thread\n";
        m_writeThread.launch( dispatch_shout, this );

        // start the worker
        std::cout << "handler " << (void*)this
                  << " launching worker thread\n";
//        m_worker.setClientId(m_clientId);
//        m_workerThread.launch( MessageHandler::dispatch_main, &m_worker );

        // create a ping message for the client
        messages::Ping* ping = new messages::Ping();
        ping->set_payload(0xdeadf00d);
        TypedMessage pingMsg(MSG_PING,ping);
        m_outboundMessages.insert(pingMsg);

        messages::Pong* pong = new messages::Pong();
        pong->set_payload(0xdeadf00d);
        TypedMessage pongMsg(MSG_PONG,pong);
        m_outboundMessages.insert(pongMsg);
    }
    catch( std::exception& ex )
    {
        std::cerr << "Exception in handler " << (void*)this << " main():\n   "
                  << ex.what();
    }

    m_readThread.join();
    std::cout << "handler " << (void*)this << " listen thread quit\n";

    // special message to notify child threads
    TypedMessage quit(MSG_QUIT);

    // put a quit message into the outgoing queue so that the shouter
    // knows its time to quit
    m_outboundMessages.insert( quit );
    m_writeThread.join();
    std::cout << "handler " << (void*)this << " shout thread quit\n";

    // put a quit message into the inbound queue so that the worker knows
    // it's time to quit
    m_inboundMessages.insert( quit );
    m_workerThread.join();
    std::cout << "handler " << (void*)this << " worker thread quit\n";

    // remove ourselves from the client map
    // m_clientMap->lockFor()->erase(m_clientId);

    // now we do our cleanup, but first lock the object so that no one tries
    // to modify us while we're doing this
    pthreads::ScopedLock lock(m_mutex);

    // invalidate peer id so any jobs that finish after we die
    // don't get sent when this handler is reused later
    m_peerId=0;

    // close the file descriptor
    close(m_sockfd);

    // generate new paramters for the next time we're run
//    std::cout << "Handler " << (void*) this
//              << " re-generating DH parameters\n";
//
//    CryptoPP::DH dh;
//    dh.AccessGroupParameters().GenerateRandomWithKeySize(m_rng,1024);
//
//    p = dh.GetGroupParameters().GetModulus();
//    q = dh.GetGroupParameters().GetSubgroupOrder();
//    g = dh.GetGroupParameters().GetGenerator();
//
//    std::cout << "Handler " << (void*) this
//              << " finished re-generating DH and returning to pool\n";

    // clear out unsent messages (todo: put these in sqlite database)
    TypedMessage msg;
    std::cout << "Handler " << (void*) this
              << " removing unsent messages\n";
    while( !m_outboundMessages.empty() )
    {
        m_outboundMessages.extract(msg);
        delete msg.msg;
    }

    // put ourselves back int the available pool
    m_pool->reassign(this);
    std::cout.flush();
    return 0;
}


void Connection::handshake()
{
    namespace msgs = messages;
    using namespace pthreads;
    using namespace CryptoPP;

    int fd[2]    = {m_sockfd,g_termNote->readFd()};

    //create message buffers
    std::cerr << "handler " << (void*) this << " is starting up"
              << std::endl;

    bool amLeader = leaderElect();
    SecByteBlock kek, mack;
    keyExchange( kek, mack );

    if(amLeader)
        sendCEK(kek,mack);
    else
        recvCEK(kek,mack);

    // now we can make our encryptor and decriptor
    m_msg.initAES(m_cek,m_iv);

    std::string base64;
    authenticatePeer(base64);

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
}

bool Connection::leaderElect()
{
    namespace msgs = messages;
    using namespace pthreads;
    int fd[2]    = {m_sockfd,g_termNote->readFd()};

    int          number = m_rng.GenerateByte();

    // first, peers generate random numbers to decide who is the leader and
    // who is the follower
    msgs::LeaderElect leaderElect;
    leaderElect.set_number( number );
    m_msg.writePlain(fd,TypedMessage(MSG_LEADER_ELECT,&leaderElect));

    TypedMessage reply = m_msg.readPlain(fd);
    validate_message(reply,MSG_LEADER_ELECT);
    msgs::LeaderElect *leaderElectRecv =
            static_cast<msgs::LeaderElect*>(reply.msg);
    bool amLeader = number > leaderElectRecv->number();
    delete leaderElectRecv;

    return amLeader;
}

void Connection::keyExchange(
        CryptoPP::SecByteBlock& kek,
        CryptoPP::SecByteBlock& mack)
{
    namespace msgs = messages;
    using namespace pthreads;
    using namespace CryptoPP;

    int fd[2]    = {m_sockfd,g_termNote->readFd()};

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

    // now, the peers send each other a key exchange message
    msgs::KeyExchange keyEx;
    keyEx.set_ekey( epub.BytePtr(), epub.SizeInBytes() );
    keyEx.set_skey( spub.BytePtr(), spub.SizeInBytes() );

    std::cout << "Sending KEY_EXCHANGE message " << std::endl;
    m_msg.writePlain( fd, TypedMessage(MSG_KEY_EXCHANGE,&keyEx) );

    std::cout << "Waiting for KEY_EXCHANGE message " << std::endl;
    TypedMessage reply = m_msg.readPlain(fd);
    validate_message( reply, MSG_KEY_EXCHANGE );

    msgs::KeyExchange* keyExRecv =
            static_cast<msgs::KeyExchange*>(reply.msg);

    // read client keys
    SecByteBlock epubClient(
                    (unsigned char*)&keyExRecv->ekey()[0],
                    keyExRecv->ekey().size() );
    SecByteBlock spubClient(
                    (unsigned char*)&keyExRecv->skey()[0],
                    keyExRecv->skey().size() );

    // delete the message
    delete keyExRecv;

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
    kek = SecByteBlock(
            shared.BytePtr(), AES::DEFAULT_KEYLENGTH);

    // CMAC key follows the 'n' bits used for KEK
    mack = SecByteBlock(
            &shared.BytePtr()[AES::DEFAULT_KEYLENGTH],
                                AES::BLOCKSIZE);
}

void Connection::sendCEK( CryptoPP::SecByteBlock& kek,
                          CryptoPP::SecByteBlock& mack)
{
    namespace msgs = messages;
    using namespace pthreads;
    using namespace CryptoPP;

    int fd[2]    = {m_sockfd,g_termNote->readFd()};

    CMAC<AES> cmac(mack.BytePtr(), mack.SizeInBytes());

    // Generate a random CEK and IV
    m_cek = SecByteBlock(AES::DEFAULT_KEYLENGTH);
    m_iv  = SecByteBlock(AES::BLOCKSIZE);
    m_rng.GenerateBlock(m_cek.BytePtr(), m_cek.SizeInBytes());
    m_rng.GenerateBlock( m_iv.BytePtr(), m_iv.SizeInBytes());

    // initialize message buffer encoder, decoder
    m_msg.initAES(m_cek,m_iv);

    // print it for checking
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
    m_msg.write(fd,MSG_CEK);
}


void Connection::recvCEK( CryptoPP::SecByteBlock& kek,
                          CryptoPP::SecByteBlock& mack)
{
    namespace msgs = messages;
    using namespace pthreads;
    using namespace CryptoPP;

    int fd[2]    = {m_sockfd,g_termNote->readFd()};

    CMAC<AES> cmac(mack.BytePtr(), mack.SizeInBytes());

    // extract and decode the content key
    // AES in ECB mode is fine - we're encrypting 1 block, so we don't need
    // padding
    ECB_Mode<AES>::Decryption aes;
    aes.SetKey(kek.BytePtr(), kek.SizeInBytes());

    // receive the content encryption key from the server
    TypedMessage recv = m_msg.readPlain(fd);
    validate_message( recv, MSG_CEK );

    // verify message sizes
    msgs::ContentKey* contentKey =
            static_cast<msgs::ContentKey*>( m_msg[MSG_CEK] );
    if( contentKey->key().size() != AES::BLOCKSIZE )
        ex()() << "Message error: CEK key size "
               << contentKey->key().size() << " is incorrect, should be "
               << AES::BLOCKSIZE;

    if( contentKey->iv().size() != AES::BLOCKSIZE )
        ex()() << "Message error: CEK iv size "
               << contentKey->iv().size() << " is incorrect, should be "
               << AES::BLOCKSIZE;

    if( contentKey->cmac().size() != AES::BLOCKSIZE )
        ex()() << "Message error: CEK cmac size "
               << contentKey->cmac().size() << " is incorrect, should be "
               << AES::BLOCKSIZE;

    // Enc(CEK)|Enc(iv)
    SecByteBlock msg_cipher( 2*AES::BLOCKSIZE );
    memcpy( msg_cipher.BytePtr(), &contentKey->key()[0], AES::BLOCKSIZE );
    memcpy( &msg_cipher.BytePtr()[AES::BLOCKSIZE],
                                  &contentKey->iv()[0], AES::BLOCKSIZE );

    // Enc(CEK)
    SecByteBlock key_cipher(
                (unsigned char* )&contentKey->key()[0], AES::BLOCKSIZE );

    // ENC(IV)
    SecByteBlock iv_cipher (
                (unsigned char*)&contentKey->iv()[0], AES::BLOCKSIZE);

    // CMAC(Enc(CEK))
    SecByteBlock msg_cmac(
                (unsigned char* )&contentKey->cmac()[0], AES::BLOCKSIZE );

    // recompute cmac for verification
    SecByteBlock chk_cmac( AES::BLOCKSIZE );
    cmac.CalculateTruncatedDigest( chk_cmac.BytePtr(), AES::BLOCKSIZE,
                                   msg_cipher.BytePtr(), 2*AES::BLOCKSIZE );
    if( chk_cmac != msg_cmac )
        ex()() << "WOAH!!! CEK message digest doesn't match, possible "
                  "tampering of data";

    // decrypt the CEK
    m_cek = SecByteBlock( AES::DEFAULT_KEYLENGTH );
    m_iv  = SecByteBlock( AES::BLOCKSIZE );
    aes.ProcessData( m_cek.BytePtr(), key_cipher.BytePtr(), AES::DEFAULT_KEYLENGTH );
    aes.ProcessData(  m_iv.BytePtr(),  iv_cipher.BytePtr(), AES::BLOCKSIZE );

    Integer cekOut, ivOut;
    cekOut.Decode( m_cek.BytePtr(), m_cek.SizeInBytes() );
    ivOut .Decode(  m_iv.BytePtr(),  m_iv.SizeInBytes() );
    std::cout << "AES Key: " << std::hex << cekOut << std::endl;
    std::cout << "     iv: " << std::hex <<  ivOut << std::endl;
    std::cout << std::dec;
}


void Connection::authenticatePeer(std::string& base64)
{
    namespace msgs = messages;
    using namespace pthreads;
    using namespace CryptoPP;

    int fd[2]    = {m_sockfd,g_termNote->readFd()};

    // trade public keys
    TypedMessage recv = m_msg.readEnc(fd);
    validate_message( recv, MSG_AUTH_REQ );

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

    msgs::AuthChallenge challenge;
    challenge.set_type( msgs::AuthChallenge::AUTHENTICATE );
    challenge.set_challenge(cipherChallenge);

    m_msg.writeEnc( fd, TypedMessage(MSG_AUTH_CHALLENGE,&challenge));

    // read the challenge solution
    recv = m_msg.readEnc(fd);
    validate_message( recv, MSG_AUTH_SOLN );
    msgs::AuthSolution* authSoln =
            static_cast<msgs::AuthSolution*>( recv.msg);
    msgs::AuthResult authResult;
    if( authSoln->solution().compare(plainChallenge) != 0 )
    {
        authResult.set_response(false);
        m_msg.writeEnc(fd,TypedMessage(MSG_AUTH_RESULT,&authResult));
        ex()() << "Client does not own the key";
    }

    authResult.set_response(true);
    m_msg.writeEnc(fd,TypedMessage(MSG_AUTH_RESULT,&authResult));

    std::cout << "Client authenticated" << std::endl;

    // base64 encode the clients public key
    queue.Clear();
    CryptoPP::Base64Encoder encoder;
    clientKey.Save(queue);
    queue.CopyTo(encoder);
    encoder.MessageEnd();
    base64.resize( encoder.MaxRetrievable(), '\0' );
    encoder.Get( (byte*)&base64[0], base64.size() );
}

void* Connection::listen()
{
    using namespace CryptoPP;
    std::cout << "Starting client listener for handler" << (void*)this
              << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    int fd[2]    = {m_sockfd,g_termNote->readFd()};

    try
    {
        // infinite loop until exception is thrown
        while(1)
        {
            // read one message from client, exception thrown on disconnect
            // or termination signal
            TypedMessage msg = m_msg.readEnc(fd);

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



void* Connection::shout()
{
    using namespace CryptoPP;
    std::cout << "Starting client shouter for handler" << (void*)this
              << " in thread "
              << pthreads::Thread::self().c_obj() << "\n";

    int fd[2]    = {m_sockfd,g_termNote->readFd()};

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
            m_msg.writeEnc(fd,msg);
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






} // namespace filesystem
} // namespace openbook

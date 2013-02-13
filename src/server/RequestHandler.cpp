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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/RequestHandler.cpp
 *
 *  @date   Feb 8, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <errno.h>
#include <sstream>

#include <protobuf/message.h>
#include <protobuf/io/zero_copy_stream_impl.h>

#include <crypto++/files.h>
#include <crypto++/cmac.h>
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/gcm.h>

#include "RequestHandler.h"
#include "messages.h"
#include "messages.pb.h"

namespace   openbook {
namespace filesystem {


extern "C"
{
    // calls RequestHandler::init
    void* request_handler_initDH( void* vp_handler )
    {
        RequestHandler* h = static_cast<RequestHandler*>(vp_handler);
        h->initDH();
        return 0;
    }
}



void RequestHandler::cleanup()
{
    close(m_fd[0]);

    std::cout << "Handler " << (void*) this
              << " re-generating DH parameters\n";

    using namespace CryptoPP;
    m_dh.AccessGroupParameters().GenerateRandomWithKeySize(m_rng,1024);
    m_dh2.GenerateStaticKeyPair(m_rng,m_spriv,m_spub);
    m_dh2.GenerateEphemeralKeyPair(m_rng,m_epriv, m_epub);

    std::cout << "Handler " << (void*) this
              << " finished re-generating DH and returning to pool\n";
    m_pool->reassign(this);
    std::cout.flush();
}

RequestHandler::RequestHandler():
    m_pool(0),
    m_fd(2),
    m_dh2(m_dh)
{
    m_mutex.init();
}

RequestHandler::~RequestHandler()
{
    m_mutex.destroy();
}

void RequestHandler::init(Pool_t* pool)
{
    using namespace pthreads;
    m_pool = pool;

    std::cout << "Initializing handler " << (void*)this << std::endl;
    ScopedLock lock(m_mutex);

    Attr<Thread> attr;
    attr.init();
    attr << DETACHED;
    int result = m_thread.launch(attr,request_handler_initDH,this);
    attr.destroy();

    if( result )
    {
        std::cerr << "Failed to start handler init thread: ";
        switch(result)
        {
            case EAGAIN:
                std::cerr << "EAGAIN";
                break;

            case EINVAL:
                std::cerr << "EINVAL";
                break;

            case EPERM:
                std::cerr << "EPERM";
                break;

            default:
                std::cerr << "unexpected errno";
                break;
        }
        std::cerr << std::endl;
        m_pool->reassign(this);
    }
}

void RequestHandler::initDH()
{
    pthreads::ScopedLock lock(m_mutex);
    std::cout << "Handler " << (void*) this
              << " generating DH parameters\n";

    using namespace CryptoPP;
    m_dh.AccessGroupParameters().GenerateRandomWithKeySize(m_rng,1024);
    m_spriv = SecByteBlock( m_dh2.StaticPrivateKeyLength() );
    m_spub  = SecByteBlock( m_dh2.StaticPublicKeyLength() );
    m_epriv = SecByteBlock( m_dh2.EphemeralPrivateKeyLength() );
    m_epub  = SecByteBlock( m_dh2.EphemeralPublicKeyLength() );

    m_dh2.GenerateStaticKeyPair(m_rng,m_spriv,m_spub);
    m_dh2.GenerateEphemeralKeyPair(m_rng,m_epriv, m_epub);
    m_shared= SecByteBlock( m_dh2.AgreedValueLength() );

    std::cout << "Handler " << (void*) this
              << " finished generating DH and returning to pool\n";
    m_pool->reassign(this);

    std::cout.flush();
}




void RequestHandler::start( int sockfd, int termfd )
{
    // lock scope
    {
        using namespace pthreads;
        ScopedLock lock(m_mutex);
        m_fd[0] = sockfd;
        m_fd[1] = termfd;
        m_fd.setTimeout(5,0);
        m_fd.init();

        Attr<Thread> attr;
        attr.init();
        attr << DETACHED;
        int result = m_thread.launch(attr,this);
        attr.destroy();

        if( result )
        {
            std::cerr << "Failed to start handler thread: ";
            switch(result)
            {
                case EAGAIN:
                    std::cerr << "EAGAIN";
                    break;

                case EINVAL:
                    std::cerr << "EINVAL";
                    break;

                case EPERM:
                    std::cerr << "EPERM";
                    break;

                default:
                    std::cerr << "unexpected errno";
                    break;
            }
            std::cerr << std::endl;
            m_pool->reassign(this);
        }
    }
}

void* RequestHandler::operator()()
{
    namespace cryp = CryptoPP;
    namespace msgs = messages;
    using namespace pthreads;
    ScopedLock lock(m_mutex);

    int received = 0;

    //create message buffers
    std::cerr << "handler " << (void*) this << " is starting up"
              << std::endl;

    try
    {

    // first the server sends the diffie hellman parameters so we can do a
    // key exchange
    std::string p,q,g;
    p.resize( m_dh.GetGroupParameters().GetModulus().ByteCount());
    g.resize( m_dh.GetGroupParameters().GetGenerator().ByteCount());
    q.resize( m_dh.GetGroupParameters().GetSubgroupOrder().ByteCount());

    m_dh.GetGroupParameters().GetModulus().Encode(
            (unsigned char*)&p[0], p.size(), cryp::Integer::UNSIGNED );
    m_dh.GetGroupParameters().GetGenerator().Encode(
            (unsigned char*)&g[0], g.size(), cryp::Integer::UNSIGNED );
    m_dh.GetGroupParameters().GetSubgroupOrder().Encode(
            (unsigned char*)&q[0], q.size(), cryp::Integer::UNSIGNED );

    msgs::DiffieHellmanParams* dhParams =
            static_cast<msgs::DiffieHellmanParams*>(m_msg[MSG_DH_PARAMS]);
    dhParams->set_p(p);
    dhParams->set_q(q);
    dhParams->set_g(g);

    std::cout << "Sending DH_PARAMS message " << std::endl;
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
    cryp::SecByteBlock epubClient(
                    (unsigned char*)&keyEx->ekey()[0], keyEx->ekey().size() );
    cryp::SecByteBlock spubClient(
                    (unsigned char*)&keyEx->skey()[0], keyEx->skey().size() );

    // write out our keys
    keyEx->set_ekey( m_epub.BytePtr(), m_epub.SizeInBytes() );
    keyEx->set_skey( m_spub.BytePtr(), m_spub.SizeInBytes() );
    std::cout << "Sending KEY_EXCHANGE message " << std::endl;
    m_msg.write( m_fd, MSG_KEY_EXCHANGE );

    // generate shared key
    if( !m_dh2.Agree(m_shared,m_spriv,m_epriv,spubClient,epubClient) )
        ex()() << "Failed to agree on a shared key";

    cryp::Integer sharedOut;
    sharedOut.Decode(m_shared.BytePtr(),m_shared.SizeInBytes() );
    std::cout << "Shared secret (client): "
              << "\n   shared: " << std::hex << sharedOut << std::endl;

    // use shared secret to generate real keys
    // Take the leftmost 'n' bits for the KEK
    cryp::SecByteBlock kek(
            m_shared.BytePtr(), cryp::AES::DEFAULT_KEYLENGTH);

    // CMAC key follows the 'n' bits used for KEK
    cryp::SecByteBlock mack(
            &m_shared.BytePtr()[cryp::AES::DEFAULT_KEYLENGTH],
                                cryp::AES::BLOCKSIZE);
    cryp::CMAC<cryp::AES> cmac(mack.BytePtr(), mack.SizeInBytes());

    // Generate a random CEK and IV
    cryp::SecByteBlock cek(cryp::AES::DEFAULT_KEYLENGTH);
    cryp::SecByteBlock iv (cryp::AES::BLOCKSIZE);
    m_rng.GenerateBlock(cek.BytePtr(), cek.SizeInBytes());
    m_rng.GenerateBlock(iv.BytePtr(), iv.SizeInBytes());

    // print it for checkin
    cryp::Integer cekOut, ivOut;
    cekOut.Decode( cek.BytePtr(), cek.SizeInBytes() );
    ivOut .Decode(  iv.BytePtr(),  iv.SizeInBytes() );
    std::cout << "AES Key: " << std::hex << cekOut << std::endl;
    std::cout << "     iv: " << std::hex << ivOut  << std::endl;
    std::cout << std::dec;

    // AES in ECB mode is fine - we're encrypting 1 block, so we don't need
    // padding
    cryp::ECB_Mode<cryp::AES>::Encryption aes;
    aes.SetKey(kek.BytePtr(), kek.SizeInBytes());

    cryp::SecByteBlock msg_cipher( 2*cryp::AES::BLOCKSIZE );  //< Enc(CEK) | Enc(iv)
    cryp::SecByteBlock   msg_cmac(   cryp::AES::BLOCKSIZE );  //< CMAC(Enc(CEK||iv))

    aes.ProcessData(msg_cipher.BytePtr(), cek.BytePtr(), cryp::AES::BLOCKSIZE );
    aes.ProcessData(&msg_cipher.BytePtr()[cryp::AES::BLOCKSIZE],
                                          iv.BytePtr(), cryp::AES::BLOCKSIZE );
    cmac.CalculateTruncatedDigest(msg_cmac.BytePtr(), cryp::AES::BLOCKSIZE,
                                  msg_cipher.BytePtr(), 2*cryp::AES::BLOCKSIZE );

    // fill the content key message
    msgs::ContentKey* contentKey =
            static_cast<msgs::ContentKey*>( m_msg[MSG_CEK] );

    contentKey->set_key( msg_cipher.BytePtr(), cryp::AES::BLOCKSIZE );
    contentKey->set_iv(  &msg_cipher.BytePtr()[ cryp::AES::BLOCKSIZE],
                                               cryp::AES::BLOCKSIZE );
    contentKey->set_cmac( msg_cmac.BytePtr(), cryp::AES::BLOCKSIZE );

    // send the content key message
    std::cout << "Sending CEK message " << std::endl;
    m_msg.write(m_fd,MSG_CEK);

    // now we can make our encryptor and decriptor
    cryp::GCM<cryp::AES>::Encryption enc;
    cryp::GCM<cryp::AES>::Decryption dec;
    enc.SetKeyWithIV(cek.BytePtr(), cek.SizeInBytes(),
                     iv.BytePtr(), iv.SizeInBytes());
    dec.SetKeyWithIV(cek.BytePtr(), cek.SizeInBytes(),
                    iv.BytePtr(), iv.SizeInBytes());

    // read the client's public key
    sleep(2);
    // this works
    // type = m_msg.read(m_fd[0],dec);

    // this doesn't, but why??
    type = m_msg.read(m_fd,dec);

    dec.Resynchronize(iv.BytePtr(), iv.SizeInBytes());

    if( type != MSG_AUTH_REQ )
        ex()() << "Protocol Error: expected AUTH_REQ from client, instead got"
               << messageIdToString(type) << "(" << (int)type << ")";

    msgs::AuthRequest* authReq =
            static_cast<msgs::AuthRequest*>( m_msg[MSG_AUTH_REQ] );
    std::stringstream  inkey( authReq->public_key() );
    cryp::FileSource keyFile(inkey,true);
    cryp::ByteQueue  queue;
    keyFile.TransferTo(queue);
    queue.MessageEnd();
    cryp::RSA::PublicKey clientKey;
    clientKey.Load(queue);

    // create an RSA Encryptor to verify ownership
    cryp::RSAES_OAEP_SHA_Encryptor rsaEnc( clientKey );

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

    m_msg.write(m_fd,MSG_AUTH_CHALLENGE,enc);
    enc.Resynchronize(iv.BytePtr(), iv.SizeInBytes());

    // read the challenge solution
    type = m_msg.read(m_fd,dec);
    dec.Resynchronize(iv.BytePtr(), iv.SizeInBytes());
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

        ex()() << "Client does not own the key";
    }

    authResult->set_response(true);
    m_msg.write(m_fd,MSG_AUTH_RESULT,enc);
    enc.Resynchronize(iv.BytePtr(), iv.SizeInBytes());
    std::cout << "Client authenticated" << std::endl;

    ex()() << "Exiting Early";

    int authRetry = 0;
    for(; authRetry < 3; authRetry++)
    {
        break;
        msgs::AuthChallenge* challenge =
            static_cast<msgs::AuthChallenge*>( m_msg[MSG_AUTH_CHALLENGE] );

        // for now, let's pretend that all clients are authorized and we'll
        // just verify that they are the key owner
        challenge->set_challenge(cipherChallenge);
        challenge->set_type( msgs::AuthChallenge::AUTHENTICATE );

        break;
    }

    // if too many retries
    if( authRetry >= 3 )
    {
        std::cout << "client failed too many times" << std::endl;
        msgs::AuthResult* result =
                static_cast<msgs::AuthResult*>( m_msg[MSG_AUTH_REQ] );
        result->set_response(false);

        cleanup();
        return 0;
    }

    while(1)
    {
        ex()() << "Exiting loop";
    }

    }
    catch ( std::exception& ex )
    {
        std::cerr << ex.what() << std::endl;
        cleanup();
        return 0;
    }

    return 0;
}











} // namespace filesystem
} // namespace openbook

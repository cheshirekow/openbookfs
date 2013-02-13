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


#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include <dirent.h>
#include <sys/time.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>
#include <fstream>
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

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

#include "Bytes.h"
#include "MessageBuffer.h"
#include "messages.h"
#include "messages.pb.h"

using namespace openbook::filesystem;

int main(int argc, char** argv)
{
    namespace fs   = boost::filesystem;
    namespace cryp = CryptoPP;
    namespace msgs = messages;

    std::string pubKey;
    std::string privKey;
    std::string host;

    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

    time_t      rawtime;
    tm*         timeinfo;
    char        currentYear[5];

    ::time( &rawtime );
    timeinfo = ::localtime( &rawtime );
    strftime (currentYear,5,"%Y",timeinfo);

    fs::path homeDir     = getenv("HOME");
    fs::path dfltPubKey  = "./rsa-openssl-pub.der";
    fs::path dfltPrivKey = "./rsa-openssl-priv.der";

    std::string dfltServer = "localhost:3031";

    std::stringstream sstream;
    sstream << "Openbook Filesystem\n"
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
    TCLAP::ValueArg<std::string> pubKeyArg(
            "b",
            "pubkey",
            "path to the ssh public key",
            false,
            dfltPubKey.string(),
            "path"
            );

    TCLAP::ValueArg<std::string> privKeyArg(
            "v",
            "privkey",
            "path to the ssh private key",
            false,
            dfltPrivKey.string(),
            "path"
            );

    TCLAP::ValueArg<std::string> serverArg(
            "s",
            "server",
            "hostname and port of the server",
            false,
            dfltServer,
            "string [HOST]:[PORT]"
            );

    // Add the argument nameArg to the CmdLine object. The CmdLine object
    // uses this Arg to parse the command line.
    cmd.add( pubKeyArg );
    cmd.add( privKeyArg );
    cmd.add( serverArg );

    // Parse the argv array.
    cmd.parse( argc, argv );

    // Get the value parsed by each arg.
    pubKey  = pubKeyArg.getValue();
    privKey = privKeyArg.getValue();
    host    = serverArg.getValue();

    }

    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "error: " << e.error() << " for arg "
                    << e.argId() << std::endl;
        return 1;
    }

    // attempt to open the public and private key files and read them in
    if( !fs::exists( fs::path(pubKey) ) )
    {
        std::cerr << "public key file: " << pubKey << " does not exist\n";
        return 1;
    }



    std::string             rsaPubStr;
    cryp::RSA::PublicKey    rsaPubKey;
    cryp::RSA::PrivateKey   rsaPrivKey;
    cryp::AutoSeededRandomPool  rng;
    try
    {
        // open a stream to the public key file
        std::ifstream in(pubKey.c_str(), std::ios::in | std::ios::binary);
        if (!in)
            ex()() << "Failed to open " << pubKey << " for reading ";

        // seek to the end of the file to get it's size
        in.seekg(0, std::ios::end);

        // resize the storage space
        rsaPubStr.resize((unsigned int)in.tellg(),'\0');

        // seek back to the beginning
        in.seekg(0, std::ios::beg);

        // read in the entire file
        in.read(&rsaPubStr[0], rsaPubStr.size());

        // seek back to the beginning again
        in.seekg(0, std::ios::beg);

        // read into public key
        cryp::FileSource keyFile(in,true);
        cryp::ByteQueue  queue;
        keyFile.TransferTo(queue);
        queue.MessageEnd();

        rsaPubKey.Load(queue);
    }
    catch( cryp::Exception& ex )
    {
        std::cerr << "Failed to load public key from " << pubKey
                  <<  " : " << ex.what() << std::endl;
        return 1;
    }

    try
    {
        cryp::FileSource keyFile(privKey.c_str(),true);
        cryp::ByteQueue  queue;
        keyFile.TransferTo(queue);
        queue.MessageEnd();

        rsaPrivKey.Load(queue);
    }
    catch( cryp::Exception& ex )
    {
        std::cerr << "Failed to load private key from " << privKey
                  <<  " : " << ex.what() << std::endl;
        return 1;
    }

    if( !rsaPubKey.Validate(rng,3) )
    {
        std::cerr << "Failed to validate public key" << std::endl;
        return 1;
    }
    if( !rsaPrivKey.Validate(rng,3) )
    {
        std::cerr << "Failed to validate private key" << std::endl;
        return 1;
    }

    // Create the TCP socket
    int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( sockfd < 0 )
    {
        std::cerr << "Failed to open tcp socket" << std::endl;
        return 1;
    }

    // parse the server string
    std::string hostName;
    int         hostPort;
    RE2         serverRegex( "([^:]+):(\\d+)" );

    if( !serverRegex.ok() )
    {
        std::cerr << "Something is wrong with the regex: ([^:]+):(\\d+)"
                  << std::endl;
        return 1;
    }

    bool matchResult =
            RE2::FullMatch(host.c_str(),serverRegex,&hostName,&hostPort);
    if( !matchResult )
    {
        std::cerr << "Failed to match [HOST]:[PORT] in string " << host
                  << std::endl;
        return 1;
    }

    hostent* hostInfo;
    in_addr  hostAddr;

    if( !inet_aton(hostName.c_str(),&hostAddr) )
    {
        hostInfo        = gethostbyname(hostName.c_str());
        hostAddr.s_addr = * ( (unsigned long*)hostInfo->h_addr_list[0] );
    }

    Bytes<in_addr_t> hbytes( &hostAddr.s_addr );
    std::cout << "Host address: "
              << hbytes[0] << "."
              << hbytes[1] << "."
              << hbytes[2] << "."
              << hbytes[3] << "\n";


    sockaddr_in         server;

    memset(&server, 0, sizeof(server));         //< Clear struct
    server.sin_family       = AF_INET;          //< Internet/IP
    server.sin_addr         = hostAddr;         //< IP address
    server.sin_port         = htons(hostPort);  //< server port

    const unsigned int  bufsize = 1024;
    char                buffer[bufsize];
    unsigned int        len;
    int                 received = 0;

    // Establish connection
    int connectResult = connect( sockfd, (sockaddr*)&server, sizeof(server) );
    if( connectResult < 0 )
    {
        std::cerr << "Failed to open connection to host "
                  << hostName << " on port " << hostPort << std::endl;
        return 1;
    }

    try
    {

    MessageBuffer msg;  //< rpc middle man
    char          type; //< type of reponse message

    // first we receive a DH parameter messages
    type = msg.read(sockfd);
    if( type != MSG_DH_PARAMS )
        ex()() << "Protocol error, first message received from server is "
               << messageIdToString(type) << "(" << (int)type << "), "
               << "expecting DH_PARAMS";

    msgs::DiffieHellmanParams* dhParams =
            static_cast<msgs::DiffieHellmanParams*>( msg[MSG_DH_PARAMS] );

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
    if( !dh.AccessGroupParameters().Validate(rng,3) )
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

        dh2.GenerateStaticKeyPair(rng,spriv,spub);
        dh2.GenerateEphemeralKeyPair(rng,epriv, epub);
        shared= SecByteBlock( dh2.AgreedValueLength() );
        std::cout << "Finished generating DH parameters" << std::endl;
    }

    // the first message is a DH key exchange
    msgs::KeyExchange* keyEx =
            static_cast<msgs::KeyExchange*>( msg[MSG_KEY_EXCHANGE] );
    keyEx->set_skey(spub.BytePtr(),spub.SizeInBytes());
    keyEx->set_ekey(epub.BytePtr(),epub.SizeInBytes());

    // send unencrypted key exchange
    msg.write(sockfd,MSG_KEY_EXCHANGE);
    type = msg.read(sockfd);

    if( type != MSG_KEY_EXCHANGE )
    {
        std::cerr << "Protocol error: expected KEY_EXCHANGE message, got "
                  << messageIdToString(type) << std::endl;
        return 1;
    }

    // read the servers keys
    cryp::SecByteBlock epubServer(
                    (unsigned char*)&keyEx->ekey()[0], keyEx->ekey().size() );
    cryp::SecByteBlock spubServer(
                    (unsigned char*)&keyEx->skey()[0], keyEx->skey().size() );

    // generate shared key
    if (!dh2.Agree(shared,spriv,epriv,spubServer,epubServer))
    {
        std::cerr << "Failed to agree on a shared key" << std::endl;
        return 1;
    }

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
    type = msg.read(sockfd);
    if( type != MSG_CEK )
        ex()() << "Protocol error: expected CEK message, instead got "
               << messageIdToString(type) << "(" << (int)type << ") ";

    // verify message sizes
    msgs::ContentKey* contentKey =
            static_cast<msgs::ContentKey*>( msg[MSG_CEK] );
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
    cryp::SecByteBlock cek( cryp::AES::DEFAULT_KEYLENGTH );
    cryp::SecByteBlock  iv( cryp::AES::BLOCKSIZE );
    aes.ProcessData( cek.BytePtr(), key_cipher.BytePtr(), cryp::AES::DEFAULT_KEYLENGTH );
    aes.ProcessData(  iv.BytePtr(),  iv_cipher.BytePtr(), cryp::AES::BLOCKSIZE );

    cryp::Integer cekOut, ivOut;
    cekOut.Decode( cek.BytePtr(), cek.SizeInBytes() );
    ivOut .Decode(  iv.BytePtr(),  iv.SizeInBytes() );
    std::cout << "AES Key: " << std::hex << cekOut << std::endl;
    std::cout << "     iv: " << std::hex <<  ivOut << std::endl;
    std::cout << std::dec;

    // now we can make our encryptor and decryptor
    cryp::GCM<cryp::AES>::Encryption enc;
    cryp::GCM<cryp::AES>::Decryption dec;
    enc.SetKeyWithIV(cek.BytePtr(), cek.SizeInBytes(),
                     iv.BytePtr(), iv.SizeInBytes());
    dec.SetKeyWithIV(cek.BytePtr(), cek.SizeInBytes(),
                    iv.BytePtr(), iv.SizeInBytes());

    // the next message we send is the authentication message carrying our
    // public key
    msgs::AuthRequest* authReq =
            static_cast<msgs::AuthRequest*>(msg[MSG_AUTH_REQ]);
    authReq->set_public_key(rsaPubStr);

    msg.write(sockfd,MSG_AUTH_REQ,enc);
    enc.Resynchronize(iv.BytePtr(), iv.SizeInBytes());

    // we expect a challenge
    type = msg.read(sockfd,dec);
    dec.Resynchronize(iv.BytePtr(), iv.SizeInBytes());
    if( type != MSG_AUTH_CHALLENGE )
        ex()() << "Protocol error: expected AUTH_CHALLENGE but received "
               << messageIdToString(type) << " (" << (int)type << ")";

    std::string solution;
    msgs::AuthChallenge* authChallenge =
            static_cast<msgs::AuthChallenge*>(msg[MSG_AUTH_CHALLENGE]);

    if( authChallenge->type() != msgs::AuthChallenge::AUTHENTICATE )
        ex()() << "Protocol error: expected an authentication challenge";

    // create an RSA Decryptor to verify ownership
    cryp::RSAES_OAEP_SHA_Decryptor rsaDec( rsaPrivKey );
    solution.resize( rsaDec.FixedMaxPlaintextLength() );
    rsaDec.Decrypt(rng,
                    (unsigned char*)&authChallenge->challenge()[0],
                    authChallenge->challenge().size(),
                    (unsigned char*)&solution[0] );

    msgs::AuthSolution* authSoln =
            static_cast<msgs::AuthSolution*>(msg[MSG_AUTH_SOLN]);
    authSoln->set_solution(solution);

    msg.write(sockfd,MSG_AUTH_SOLN,enc);
    enc.Resynchronize(iv.BytePtr(), iv.SizeInBytes());

    type = msg.read(sockfd,dec);
    dec.Resynchronize(iv.BytePtr(), iv.SizeInBytes());
    if( type != MSG_AUTH_RESULT )
        ex()() << "Protocol Error: expected AUTH_RESULT but got "
               << messageIdToString(type) << " (" << (int)type << ")";

    msgs::AuthResult* authResult =
            static_cast<msgs::AuthResult*>( msg[MSG_AUTH_RESULT] );
    if( !authResult->response() )
        ex()() << "Failed the servers challenge";

    // now we expect a authorization challenge
    type = msg.read(sockfd,dec);
    dec.Resynchronize(iv.BytePtr(), iv.SizeInBytes());

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

    msg.write(sockfd,MSG_AUTH_SOLN,enc);
    enc.Resynchronize(iv.BytePtr(), iv.SizeInBytes() );

    // read response
    type = msg.read(sockfd,dec);
    dec.Resynchronize(iv.BytePtr(), iv.SizeInBytes() );

    if( type != MSG_AUTH_RESULT )
        ex()() << "Protocol Error: expected AUTH_RESULT but got "
               << messageIdToString(type) << " (" << (int)type << ")";

    if( authResult->response() )
        std::cout << "Authorized!!" << std::endl;
    else
        ex()() << "Not authorized";

    sleep(1);

    }
    catch( std::exception& ex )
    {
        std::cerr << ex.what() << std::endl;
        sleep(1);
    }

    // close the socket
    close(sockfd);



}





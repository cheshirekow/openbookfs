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
 *  @file   /home/josh/Codes/cpp/openbookfs/test/diffie_hellman_test.cpp
 *
 *  @date   Feb 12, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <crypto++/osrng.h>
#include <crypto++/dh.h>
#include <crypto++/dh2.h>
#include <iostream>

int main()
{
    using namespace CryptoPP;

    AutoSeededRandomPool rng;

    DH                dhA;         //< Diffie-Hellman structure
    DH2               dh2A(dhA);   //< Diffie-Hellman structure
    SecByteBlock      sprivA;      //< static private key
    SecByteBlock      spubA;       //< static public key
    SecByteBlock      eprivA;      //< ephemeral private key
    SecByteBlock      epubA;       //< ephemeral public key
    SecByteBlock      sharedA;     //< shared key

    DH                dhB;         //< Diffie-Hellman structure
    DH2               dh2B(dhA);   //< Diffie-Hellman structure
    SecByteBlock      sprivB;      //< static private key
    SecByteBlock      spubB;       //< static public key
    SecByteBlock      eprivB;      //< ephemeral private key
    SecByteBlock      epubB;       //< ephemeral public key
    SecByteBlock      sharedB;     //< shared key

    std::cout << "Initializing DH parameters" << std::endl;;
    dhA.AccessCryptoParameters().GenerateRandomWithKeySize(rng,1024);
    dhB.AccessCryptoParameters().GenerateRandomWithKeySize(rng,1024);

    std::cout << "Generating Keys" << std::endl;;
    sprivA = SecByteBlock( dh2A.StaticPrivateKeyLength() );
    spubA  = SecByteBlock( dh2A.StaticPublicKeyLength() );
    eprivA = SecByteBlock( dh2A.EphemeralPrivateKeyLength() );
    epubA  = SecByteBlock( dh2A.EphemeralPublicKeyLength() );

    dh2A.GenerateStaticKeyPair(rng,sprivA,spubA);
    dh2A.GenerateEphemeralKeyPair(rng,eprivA, epubA);
    sharedA= SecByteBlock( dh2A.AgreedValueLength() );

    sprivB = SecByteBlock( dh2B.StaticPrivateKeyLength() );
    spubB  = SecByteBlock( dh2B.StaticPublicKeyLength() );
    eprivB = SecByteBlock( dh2B.EphemeralPrivateKeyLength() );
    epubB  = SecByteBlock( dh2B.EphemeralPublicKeyLength() );

    dh2B.GenerateStaticKeyPair(rng,sprivB,spubB);
    dh2B.GenerateEphemeralKeyPair(rng,eprivB, epubB);
    sharedB= SecByteBlock( dh2B.AgreedValueLength() );

    if(!dh2A.Agree(sharedA, sprivA, eprivA, spubB, epubB) )
        std::cerr << "A failed to agree " << std::endl;

    if(!dh2B.Agree(sharedB, sprivB, eprivB, spubA, epubA) )
        std::cerr << "B failed to agree " << std::endl;

    Integer sharedAOut, sharedBOut;
    sharedAOut.Decode(sharedA.BytePtr(), sharedA.SizeInBytes());
    sharedBOut.Decode(sharedB.BytePtr(), sharedB.SizeInBytes());

    std::cout << "shared secret:"
              << "\n A: " << std::hex << sharedAOut
              << "\n B: " << std::hex << sharedBOut
              << std::endl;

}








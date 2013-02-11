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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/Client.cpp
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "Client.h"
#include "ExceptionStream.h"
#include <sstream>
#include <crypto++/files.h>
#include <crypto++/cryptlib.h>


namespace   openbook {
namespace filesystem {

void Client::initKey( const std::string& pubKey )
{
    try
    {
        std::stringstream keyStream(pubKey);
        CryptoPP::FileSource keyFile(keyStream,true);
        CryptoPP::ByteQueue  queue;
        keyFile.TransferTo(queue);
        queue.MessageEnd();

        m_pubKey.Load(queue);
    }
    catch( CryptoPP::Exception& cex )
    {
        ex()() << "Failed to load public key from " << pubKey
               <<  " : " << cex.what();
    }
}

void Client::encrypt( const std::string& plain,
                      std::string& cipher )
{
    using namespace CryptoPP;
    RSAES_OAEP_SHA_Encryptor e(m_pubKey);

    StringSource ss1(plain, true,
        new PK_EncryptorFilter(m_rng, e, new StringSink(cipher)
       ) // PK_EncryptorFilter
    ); // StringSource
}


} // namespace filesystem
} // namespace openbook


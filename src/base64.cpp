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
 *  @file   src/client_fs/base64.cpp
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include "base64.h"
#include <cassert>
#include <iostream>
#include <cstring>

namespace   openbook {
namespace filesystem {

bool base64Decode( const std::string& in, std::string& out )
{
    // ensure that the input string is a multiple of 4 bytes
    assert( (in.size() + 3) / 4 == in.size()/4 );

    // number of padding bytes appended to the end of the string
    int nEqual = 0;
    for(int i=in.size()-1; in[i] == '=' && i >= 0; i--)
        nEqual++;

    // size in bytes after base64 decoding
    int size = 3*in.size()/4;

    // resize the output and fill with null bytes
    out.resize(size,'\0');

    int w = 0;
    // read in sequences of 4 bytes from the input
    for(int i=0; i < in.size(); i+=4)
    {
        // four bytes encoded
        unsigned char code[4];

        for(int j=0; j < 4; j++)
        {
            if( 'A' <= in[i+j] && in[i+j] <= 'Z' )
                code[j] = in[i+j] - 'A';

            else if( 'a' <= in[i+j] && in[i+j] <= 'z' )
                code[j] = 26 + in[i+j] - 'a';

            else if( '0' <= in[i+j] && in[i+j] <= '9' )
                code[j] = 52 + in[i+j] - '0';

            else if( in[i+j] == '+' )
                code[j] = 62;

            else if( in[i+j] == '/' )
                code[j] = 63;

            else if( in[i+j] == '=' )
                code[j] = 0;
            else
            {
                std::cerr << "unexepected base64 character: " << in[i+j]
                          << "\n";
                return false;
            }
        }

        out[w]      |= code[0] << 2;
        out[w]      |= code[1] >> 4;
        out[w+1]    |= code[1] << 4;
        out[w+1]    |= code[2] >> 2;
        out[w+2]    |= code[2] << 6;
        out[w+2]    |= code[3];
        w+=3;
    }

    // remove nEqual bytes from the back
    out.resize(out.size() - nEqual);
    return true;
}


void base64Encode( const std::string& in, std::string& out )
{
    // size in bytes after base64 decoding, integer divide by 3, round up
    int size = 4*( (in.size()+2) /3 );

    // resize the output and fill with null bytes
    out.resize(size,'\0');

    int w = 0;
    // read in sequences of 4 bytes from the input
    for(int i=0; i < in.size(); i+=3)
    {
        unsigned char raw[3];   //< three bytes with 8 bit precision
        bool          pad[3];   //< whether or not it's padding
        unsigned char code[4];  //< four bytes with 6 bit precision

        for(int j=0; j < 3; j++)
        {
            int ii = i+j;
            pad[j] = ii >= in.size();
            raw[j] = pad[j] ? 0 : in[ii];
        }

        // clear out the coded bytes
        memset( code, 0, 4*sizeof(unsigned char));

        // do the bit hacking
        code[0] |= (raw[0] & 0xFC) >> 2;    //< bits 0-5
        code[1] |= (raw[0] & 0x03) << 6;    //< bits 6-7
        code[1] |= (raw[1] & 0xF0) >> 4;    //< bits 8-11
        code[2] |= (raw[1] & 0x0F) << 2;    //< bits 12-15
        code[2] |= (raw[2] & 0xC0) >> 6;    //< bits 16-17
        code[3] |= (raw[2] & 0x3F);         //< bits 18-23

        // mark extra bytes if they are extra
        for(int j=1; j < 3;j ++)
            if( pad[j] )
                code[j+1] = 0xFF;

        // map the char code to characters
        for(int j=0; j < 4; j++)
        {
            if( code[0] < 26 )
                out[i+j] = 'A' + code[j];
            else if( code[0] < 52 )
                out[i+j] = 'a' + code[j] - 26;
            else if( code[0] < 62 )
                out[i+j] = '0' + code[j] - 52;
            else if( code[0] == 62 )
                out[i+j] = '+';
            else if( code[0] == 63 )
                out[i+j] = '/';
            else
                out[i+j] = '=';
        }
    }
}



} // namespace filesystem
} // namespace openbook













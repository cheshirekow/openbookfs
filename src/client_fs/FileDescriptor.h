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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/FileDescriptor.h
 *
 *  @date   Feb 20, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FILEDESCRIPTOR_H_
#define OPENBOOK_FILEDESCRIPTOR_H_

#include <bitset>
#include <cpp-pthreads.h>


namespace   openbook {
namespace filesystem {
namespace     client {


/// flags for FileDescriptor
namespace fd
{
    enum Flags
    {
        FLAG_OPENED =0, ///< descriptor is in use
        FLAG_CHANGED,   ///< file has changed
        NUM_FLAGS
    };
}

/// provides some additional information about opened files, in particular,
/// whether or not they've actually been written to
class FileDescriptor
{
    public:
        typedef std::bitset<fd::NUM_FLAGS>  FlagBits;

    private:
        pthreads::Mutex m_mutex;    ///< not sure if this is is needed
        FlagBits        m_flags;    ///< flags

    public:
        /// initializes mutex
        FileDescriptor();

        /// destroys mutex
        ~FileDescriptor();

        /// for locking the descriptor
        pthreads::Mutex& mutex();

        /// mark as opened and reset state
        void open();

        /// get a flag state
        bool flag( fd::Flags );

        /// set a flag state
        void flag( fd::Flags, bool );
};


/// interface for statically sized array of descriptors
struct FileDescriptorArray
{
    virtual ~FileDescriptorArray(){}

    virtual FileDescriptor* operator[](unsigned int i)=0;
    virtual const FileDescriptor* operator[](unsigned int i) const=0;
};

/// statically sized array of file descriptors
template < unsigned int Size=1024 >
class FileDescriptorArrayImpl:
    public FileDescriptorArray
{
    private:
        FileDescriptor m_fd[Size];

    public:
        virtual ~FileDescriptorArrayImpl(){}

        virtual FileDescriptor* operator[](unsigned int i)
        {
            if( i < Size )
                return m_fd + i;
            else
                return 0;
        }

        virtual const FileDescriptor* operator[](unsigned int i) const
        {
            if( i < Size )
                return m_fd + i;
            else
                return 0;
        }
};





} // namespace client
} // namespace filesystem
} // namespace openbook



#endif // FILEDESCRIPTOR_H_

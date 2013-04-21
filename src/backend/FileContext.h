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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/FileContext.h
 *
 *  @date   Apr 21, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_FILECONTEXT_H_
#define OPENBOOK_FS_FILECONTEXT_H_

#include <vector>
#include <boost/filesystem.hpp>
#include <cpp-pthreads.h>

#include "MetaFile.h"
#include "ReferenceCounted.h"


namespace   openbook {
namespace filesystem {

/// information about an opened file
class FileContext:
    public ReferenceCounted
{
    public:
        typedef boost::filesystem::path Path_t;

    private:
        int         m_fd;       ///< os file descriptor
        MetaFile    m_meta;     ///< sqlite session for meta data
        bool        m_changed;  ///< set to true if there is a write

        /// create a file context for file-descriptor based operations
        FileContext( const Path_t& path, int fd );

    public:
        ~FileContext();
        int       fd()  { return m_fd; }
        MetaFile& meta(){ return m_meta; }
        void      mark(){ m_changed = true; }

        static RefPtr<FileContext> create( const Path_t& path, int fd );
};

/// maps file descriptors to FileContext structures
/**
 *  note these file descriptors are not OS file descriptors, but are specific
 *  to the openbook filesystem
 */
class FileMap
{
    public:
        typedef boost::filesystem::path     Path_t;
        typedef RefPtr<FileContext>         FilePtr_t;
        typedef std::vector<FilePtr_t>      FileVec_t;
        typedef std::vector<int>            IdxVec_t;

    private:
        pthreads::Mutex m_mutex;
        FileVec_t       m_fileVec;      ///< pointers to allocated FileContexts
        IdxVec_t        m_freeStore;    ///< list of unused file descriptors

    public:
        FileMap( int size = 100 );
        ~FileMap();

        /// retrieve a FileContext from it's file descriptor
        FilePtr_t operator[](int i);

        /// create a new FileContext for an opened file and return the
        /// file descriptor
        int registerFile( const Path_t& path, int os_fd );

        /// unreference a FileContext and free the file descriptor
        void unregisterFile( int fd );

};


} //< filesystem
} //< openbook















#endif // FILECONTEXT_H_

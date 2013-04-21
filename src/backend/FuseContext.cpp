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
 *  @file   src/backend/FuseContext.cpp
 *
 *  @date   Apr 16, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "Backend.h"
#include "FuseContext.h"
#include "MetaFile.h"

namespace   openbook {
namespace filesystem {



int FuseContext::result_or_errno(int result)
{
    if(result < 0)
        return -errno;
    else
        return result;
}

FuseContext::FuseContext(Backend* backend, const std::string& relpath)
{
    m_backend  = backend;
    m_dataDir  = backend->dataDir();
    if( relpath.length() > 0 )
        m_realRoot = backend->realRoot() / relpath;
    else
        m_realRoot = backend->realRoot();
}




FuseContext::~FuseContext()
{
}




int FuseContext::mknod (const char *path, mode_t mode, dev_t dev)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = (m_realRoot / path);
    std::cerr << "FuseContext::mknod: " << path
              << "(" << (m_realRoot / path) << ")" << std::endl;

    // we do not allow special files
    if( mode & ( S_IFCHR | S_IFBLK ) )
      return -EINVAL;

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    // create a directory to hold the file, and then a real file to hold
    // the contents
    int result = ::mkdir( wrapped.c_str(), (mode | S_IXUSR | S_IXGRP) & 0777 );
    if( result )
        return -errno;

    // create the local version of the file
    Path_t realFile = wrapped / "real";
    result = ::mknod( realFile.c_str(), mode, 0 );
    if( result )
        return -errno;

    // add an entry to the directory listing
    try
    {
        mode_t modeMask = 0777;
        mode_t typeMask = ~modeMask;
        MetaFile parentMeta( parent );
        parentMeta.mknod( filename.string(), mode & typeMask, mode & modeMask );

        // create the new meta file
        MetaFile meta( wrapped );
        meta.init();
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::mknod: "
                  << "\n path: " << path
                  << "\n real: " << wrapped
                  << "\n  err: " << ex.what()
                  << "\n";
    }


    return 0;
}




int FuseContext::create (const char *path,
                            mode_t mode,
                            struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = (m_realRoot / path);
    std::cerr << "FuseContext::create: "
             << "\n   path: " << path
             << "\n   real: " << wrapped
             << "\n   mode: " << std::hex << mode << std::dec
             << "\n";

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    // create a directory to hold the file, and then a real file to hold
    // the contents
    int result = ::mkdir( wrapped.c_str(), mode | S_IXUSR | S_IXGRP );
    if( result )
        return -errno;

    // create the local version of the file
    Path_t realFile = wrapped / "real";
    result = ::creat( realFile.c_str(), mode );
    if( result < 0 )
        return -errno;

    // create a file descriptor for the opened file
    int os_fd = result;
    int my_fd = -1;
    try
    {
        // add an entry to the directory listing
        MetaFile parentMeta( parent );
        parentMeta.mknod( filename.string(), S_IFREG, mode );

        // initialize the new meta file
        MetaFile meta( wrapped );
        meta.init();

        my_fd   = m_openedFiles.registerFile(wrapped,os_fd);
        result  = 0;
    }
    catch( const std::exception& ex )
    {
        my_fd   = -1;
        result  = -ENOMEM;
        ::close(os_fd);

        std::cerr << "FuseContext::create: "
             << "\n   path: " << path
             << "\n   real: " << wrapped
             << "\n   mode: " << std::hex << mode << std::dec
             << "\n";
    }

    fi->fh = my_fd;
    return result;
}




int FuseContext::open (const char *path, struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::open: "
              << "\n    path: " << path
              << "\n    real: " << wrapped
              << "\n";

    // first we make sure that the file exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    // make sure that the directory holding the file exists
    if( !fs::exists(wrapped) )
    {
        if( fi->flags | O_CREAT )
            return this->create(path,0777,fi);
        else
            return -EEXIST;
    }

    // open the local version of the file
    Path_t realFile = wrapped / "real";
    int result = ::open( realFile.c_str(), fi->flags );
    if( result < 0 )
        return -errno;

    // create a file descriptor for the opened file
    // create a file descriptor for the opened file
    int os_fd = result;
    int my_fd = -1;
    try
    {
        my_fd   = m_openedFiles.registerFile(wrapped,os_fd);
        result  = 0;
    }
    catch( const std::exception& ex )
    {
        my_fd   = -1;
        result  = -ENOMEM;
        ::close(os_fd);
    }

    fi->fh = my_fd;
    return result;
}





int FuseContext::read (const char *path,
                        char *buf,
                        size_t bufsize,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = (m_realRoot / path);
    std::cerr << "FuseContext::read: "
              << "\n    path: " << path
              << "\n    real: " << wrapped
              << "\n    size: " << bufsize
              << "\n     off: " << offset
              << "\n      fh: " << fi->fh
              << "\n";

    // if fi has a file handle then we simply read from the file handle
    if( fi->fh )
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if(file)
        {
            int result = ::pread(file->fd(),buf,bufsize,offset);
            if( result < 0 )
                return -errno;
            else
                return result;
        }
        else
            return -EBADF;
    }
    // otherwise we open the file and perform the read
    else
    {
        // first we make sure that the file exists
        if( !fs::exists(wrapped) )
          return -ENOTDIR;

        // make sure that the directory holding the file exists
        if( fi->flags | O_CREAT )
            return this->create(path,0777,fi);
        else
            return -EEXIST;

        // open the local version of the file
        Path_t realFile = wrapped / "real";
        int result = ::open( realFile.c_str(), fi->flags );
        if( result < 0 )
            return -errno;

        int fh = result;

        // perform the read
        result = ::pread(fh,buf,bufsize,offset);

        // close the file
        ::close(fh);

        return result_or_errno(result);
    }


}



int FuseContext::write (const char *path,
                        const char *buf,
                        size_t bufsize,
                        off_t offset,
                      struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = (m_realRoot / path);
    std::cerr << "FuseContext::write: "
              << "\n    path: " << path
              << "\n        : " << wrapped
              << "\n      fh: " << fi->fh
              << "\n";

    // if fi has a file handle then we simply read from the file handle
    if( fi->fh )
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if(file)
        {
            int result = ::pwrite(file->fd(),buf,bufsize,offset);
            if( result < 0 )
                return -errno;
            else
            {
                file->mark();
                return result;
            }
        }
        else
            return -EBADF;
    }
    // otherwise we open the file and perform the read
    else
    {
        // first we make sure that the file exists
        if( !fs::exists(wrapped) )
          return -ENOTDIR;

        // make sure that the directory holding the file exists
            if( fi->flags | O_CREAT )
                return this->create(path,0777,fi);
            else
                return -EEXIST;

        // open the local version of the file
        Path_t realFile = wrapped / "real";
        int result = ::open( realFile.c_str(), fi->flags );
        if( result < 0 )
            return -errno;

        int fh = result;

        // perform the write
        result = ::pwrite(fh,buf,bufsize,offset);

        // close the file
        ::close(fh);

        // increment the version
        MetaFile meta( wrapped );
        meta.incrementVersion();

        return result_or_errno(result);
    }
}




int FuseContext::truncate (const char *path, off_t length)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = (m_realRoot / path).string();
    std::cerr << "FuseContext::truncate: "
              << "\n      path: " << path
              << "\n          : " << wrapped
              << "\n    length: " << length
              << "\n";

    Path_t realFile = wrapped / "real";
    int result = ::truncate( realFile.c_str(), length  );
    if( result <  0 )
        return -errno;

    MetaFile meta( wrapped.parent_path() );
    meta.truncate( wrapped.filename().string(), length );

    return result;
}




int FuseContext::ftruncate (const char *path,
                            off_t length,
                            struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = (m_realRoot / path).string();
    std::cerr << "FuseContext::ftruncate: "
              << "\n   path: " << path
              << "\n       : " << (m_realRoot / path)
              << "\n    len: " << length
              << "\n     fh: " << fi->fh
              << "\n";

    if(fi->fh)
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if(file)
        {
            int result = ::ftruncate(file->fd(), length);
            if( result < 0 )
                return -errno;

            file->mark();
            return result;
        }
        else
            return -EBADF;
    }
    else
        return -EBADF;
}





int FuseContext::release (const char *path, struct fuse_file_info *fi)
{
    std::cerr << "FuseContext::release: "
              << "\n   path: " << path
              << "\n       : " << (m_realRoot / path)
              << "\n     fh: " << fi->fh
              << "\n";

    if(fi->fh)
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if(file)
        {
            int result = ::close(file->fd());
            if( result < 0 )
                return -errno;
            else
                return result;
        }
        else
            return -EBADF;
    }
    else
        return -EBADF;
}















int FuseContext::getattr (const char *path, struct stat *out)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::getattr: "
              << "\n       this: " << (void*) this
              << "\n    request: " << path
              << "\n translated: " << wrapped
              << "\n";

    Path_t realFile = wrapped / "real";
    if( fs::exists(realFile) )
    {
        int result = ::lstat( realFile.c_str(), out );
        return result_or_errno( result );
    }
    else
    {
        int result = ::lstat( wrapped.c_str(), out );
        return result_or_errno( result );
    }
}




int FuseContext::fgetattr (const char *path,
                            struct stat *out,
                            struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::fgetattr: "
              << "\n       this: " << (void*) this
              << "\n    request: " << path
              << "\n translated: " << wrapped
              << "\n";

    if( fi->fh )
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if(file)
        {
            int result = ::fstat(file->fd(),out);
            return result_or_errno(result);
        }
        else
            return -EBADF;
    }
    else
        return -EBADF;
}



int FuseContext::readlink (const char *path, char *buf, size_t bufsize)
{
    Path_t wrapped = m_realRoot / path;

    std::cerr << "FuseContext::readlink: "
              << path
              << "(" << wrapped << ")\n";

    Path_t realFile = wrapped / "real";
    return ::readlink(  realFile.c_str(), buf, bufsize ) ? -errno : 0;
}









int FuseContext::mkdir (const char *path, mode_t mode)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::mkdir: "
              << path
              << "(" << wrapped << ")\n";


    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    try
    {
        // add an entry to the directory listing
        MetaFile parentMeta( parent );
        parentMeta.mknod( filename.string(), S_IFDIR, mode );
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::mkdir: "
              << "\n path: " << path
              << "\n real: " << wrapped
              << "\n  err: " << ex.what()
              << "\n";
        return -EBADR;
    }

    // create the directory
    int result = ::mkdir( wrapped.c_str(), mode );
    if( result )
        return -errno;

    try
    {
        // create the new meta file
        MetaFile meta( wrapped );
        meta.init();
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::mkdir: "
              << "\n path: " << path
              << "\n real: " << wrapped
              << "\n  err: " << ex.what()
              << "\n";
        return -EBADR;
    }

    return 0;
}



int FuseContext::unlink (const char *path)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::unlink: "
              << path
              << "(" << wrapped << ")\n";

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    // unlink the directory holding the file contents, the meta file,
    // and the staged file
    fs::remove_all( wrapped );

    // remove the entry from the parent
    try
    {
        MetaFile parentMeta( parent );
        parentMeta.unlink( filename.string() );
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::unlink: "
                  << "\n path: " << path
                  << "\n real: " << wrapped
                  << "\n  err: " << ex.what()
                  << "\n";
    }

    return 0;
}



int FuseContext::rmdir (const char *path)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;
    std::cerr << "rmdir: "
              << path
              << "(" << wrapped << ")\n";

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    // unlink the directory holding the file contents, the meta file,
    // and the staged file
    fs::remove_all( wrapped );

    // remove the entry from the parent
    MetaFile parentMeta( parent );
    parentMeta.unlink( filename.string() );

    return 0;
}



int FuseContext::symlink (const char *oldpath, const char *newpath)
{
    Path_t oldwrap = m_realRoot / oldpath;
    Path_t newwrap = m_realRoot / newpath;

    std::cerr << "FuseContext::simlink: "
              << "\n   old: " << oldpath
              << "\n      : " << oldwrap
              << "\n   new: " << newpath
              << "\n      : " << newwrap
              << "\n";

    return result_or_errno( ::symlink( oldwrap.c_str(), newwrap.c_str() ) );
}



int FuseContext::rename (const char *oldpath, const char *newpath)
{
    Path_t oldwrap = m_realRoot / oldpath;
    Path_t newwrap = m_realRoot / newpath;

    std::cerr << "FuseContext::rename: "
              << "\n   old: " << oldpath
              << "\n      : " << oldwrap
              << "\n   new: " << newpath
              << "\n      : " << newwrap
              << "\n";

    return result_or_errno( ::rename( oldwrap.c_str(), newwrap.c_str() ) );
}



int FuseContext::link (const char *oldpath, const char *newpath)
{
    Path_t oldwrap = m_realRoot / oldpath;
    Path_t newwrap = m_realRoot / newpath;

    std::cerr << "FuseContext::link: "
              << "\n   old: " << oldpath
              << "\n      : " << oldwrap
              << "\n   new: " << newpath
              << "\n      : " << newwrap
              << "\n";

    return result_or_errno( ::link( oldwrap.c_str(), newwrap.c_str() ) );
}



int FuseContext::chmod (const char *path, mode_t mode)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::chmod: "
              << "\n    path: " << path
              << "\n        : " << wrapped
              << "\n    mode: " << mode
              << "\n";

    return result_or_errno( ::chmod( wrapped.c_str(), mode ) );
}



int FuseContext::chown (const char *path, uid_t owner, gid_t group)
{
    std::string wrapped = (m_realRoot / path).string();
    std::cerr << "FuseContext::chown: "
              << "\n    path: " << path
              << "\n        : " << (m_realRoot/path)
              << "\n     uid: " << owner
              << "\n     gid: " << group
              << "\n";

    return  result_or_errno( ::chown( wrapped.c_str(), owner, group ) );
}







//int FuseContext::utime (const char *, struct utimbuf *)
//{
//
//}









int FuseContext::statfs (const char *path, struct statvfs *buf)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::statfs: "
              << "\n   path: " << path
              << "\n   real: " << wrapped
              << "\n";

    return result_or_errno( ::statvfs( wrapped.c_str(), buf ) );
}



int FuseContext::flush (const char *path, struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::flush: "
              << "\n   path: " << path
              << "\n   real: " << wrapped
              << "\n     fh: " << fi->fh
              << "\n";
    return 0;
}







int FuseContext::fsync (const char *path,
                        int datasync,
                        struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::fsync: "
              << "\n   path: " << path
              << "\n       : " << wrapped
              << "\n     ds: " << datasync
              << "\n     fh: " << fi->fh
              << "\n";

    if(fi->fh)
    {
        if(datasync)
            return result_or_errno(
                    ::fdatasync(fi->fh)
                     );
        else
            return result_or_errno(
                    ::fsync(fi->fh)
                     );
    }
    else
        return -EBADF;
}



int FuseContext::setxattr (const char *path,
                            const char *key,
                            const char *value,
                            size_t bufsize,
                            int unknown)
{
    std::cerr << "FuseContext::setxattr" << std::endl;
    return 0;
}



int FuseContext::getxattr (const char *path,
                            const char *key,
                            char *value,
                            size_t bufsize)
{
    std::cerr << "FuseContext::getxattr" << std::endl;
    return 0;
}



int FuseContext::listxattr (const char *path, char *buf, size_t bufsize)
{
    std::cerr << "FuseContext::listxattr" << std::endl;
    return 0;
}



int FuseContext::removexattr (const char *path, const char *key)
{
    std::cerr << "FuseContext::removexattr" << std::endl;
    return 0;
}



int FuseContext::opendir (const char *path, struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    try
    {
        int fd = m_openedFiles.registerFile( wrapped, -1 );
        fi->fh = fd;
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::opendir: "
          << "\n   path: " << path
          << "\n   real: " << wrapped
          << "\n  error: " << ex.what()
          << "\n";
        return -ENOMEM;
    }

    return 0;
}



int FuseContext::readdir (const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::readdir" << std::endl;
    std::cerr << "readdir: "
                 "\n   this: " << (void*)this <<
                 "\n   path: " << path <<
                 "\n       : " << (m_realRoot / path) <<
                 "\n    off: " << offset <<
                 "\n     fh: " << fi->fh <<
                 std::endl;

    RefPtr<FileContext> file;
    if( fi->fh )
    {
        file = m_openedFiles[fi->fh];
        if(!file)
            return -EBADF;
    }
    else
    {
        file = FileContext::create(wrapped,-1);
    }

    file->meta().readdir(buf,filler,offset);
    return 0;
}



int FuseContext::releasedir (const char *path,
                                struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::releasedir: "
              << "\n   path: " << path
              << "\n   real: " << wrapped
              << "\n     fh: " << fi->fh
              << "\n";

    if(fi->fh)
        m_openedFiles.unregisterFile(fi->fh);
    else
        return -EBADF;

    return 0;
}



int FuseContext::fsyncdir (const char *path,
                            int datasync,
                            struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::syncdir: "
              << "\n   path: " << path
              << "\n       : " << wrapped
              << "\n     ds: " << datasync
              << "\n     fh: " << fi->fh
              << "\n";
    return 0;
}





int FuseContext::access (const char *path, int mode)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::access: "
              << "\n   path: " << path
              << "\n       : " << wrapped
              << "\n   mode: " << mode
              << "\n";

    return result_or_errno( ::access( wrapped.c_str(), mode ) );
}








int FuseContext::lock (const char *path,
                        struct fuse_file_info *fi,
                        int cmd,
                        struct flock *fl)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::lock: "
              << "\n   path: " << path
              << "\n       : " << wrapped
              << "\n       : " << cmd
              << "\n     fh: " << fi->fh
              << "\n";

    if( fi->fh )
    {
        return result_or_errno(
                fcntl(fi->fh,cmd,fl)
                );
    }
    else
        return -EBADF;
}



int FuseContext::utimens (const char *path, const struct timespec tv[2])
{
    std::cerr << "FuseContext::utimens: "
              << "\n   path: " << path
              << "\n       : " << (m_realRoot / path)
              << "\n     t1: " << tv[0].tv_sec << " : " << tv[0].tv_nsec
              << "\n     t2: " << tv[1].tv_sec << " : " << tv[1].tv_nsec
              << "\n";

    timeval times[2];
    for(int i=0; i < 2; i++)
    {
        times[i].tv_sec = tv[i].tv_sec;
        times[i].tv_usec= tv[i].tv_nsec / 1000;
    }

    return result_or_errno(
            ::utimes(path,times)
             );
}



int FuseContext::bmap (const char *, size_t blocksize, uint64_t *idx)
{
    std::cerr << "FuseContext::bmap" << std::endl;
    return 0;
}



int FuseContext::ioctl (const char *path,
                        int cmd,
                        void *arg,
                        struct fuse_file_info *fi,
                        unsigned int flags,
                        void *data)
{
    std::cerr << "FuseContext::ioctl" << std::endl;
    return 0;
}



int FuseContext::poll ( const char *, struct fuse_file_info *,
                      struct fuse_pollhandle *ph, unsigned *reventsp)
{
    std::cerr << "FuseContext::poll" << std::endl;
    return 0;
}




} // namespace filesystem
} // namespace openbook









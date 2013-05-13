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
#include <unistd.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "Backend.h"
#include "FuseContext.h"

namespace   openbook {
namespace filesystem {



int FuseContext::result_or_errno(int result)
{
    if(result < 0)
        return -errno;
    else
        return result;
}

FuseContext::FuseContext(Backend* backend, const std::string& relpath):
    m_openedFiles(backend)
{
    m_backend  = backend;
    m_dataDir  = backend->dataDir();
    m_relDir   = relpath;
    if( relpath.length() > 0 && relpath != "/" )
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

    // we do not allow special files
    if( mode & ( S_IFCHR | S_IFBLK ) )
      return -EINVAL;

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) || !fs::is_directory(parent) )
      return -ENOENT;

    // create the local version of the file
    int result = ::mknod( wrapped.c_str(), mode, 0 );
    if( result )
        return -errno;

    // add an entry to the directory listing
    try
    {
        mode_t modeMask = 0777;
        mode_t typeMask = ~modeMask;
        m_backend->db().mknod( Path_t(path) );
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

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) || !fs::is_directory(parent) )
      return -ENOENT;

    // create the local version of the file
    int result = ::creat( wrapped.c_str(), mode );
    if( result < 0 )
        return -errno;

    // create a file descriptor for the opened file
    int os_fd = result;
    int my_fd = -1;
    try
    {
        // add an entry to the directory listing
        m_backend->db().mknod( Path_t(path) );

        my_fd   = m_openedFiles.registerFile( Path_t(path) ,os_fd);
        result  = 0;
    }
    catch( const std::exception& ex )
    {
        my_fd   = -1;
        result  = -EIO;
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

    // first we make sure that the file exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    // make sure that the file exists, if it doesn't exist check for the
    // O_CREAT flag
    if( !fs::exists(wrapped) )
    {
        if( fi->flags | O_CREAT )
            return this->create(path,0777,fi);
        else
            return -EEXIST;
    }

    // open the file
    int result = ::open( wrapped.c_str(), fi->flags );
    if( result < 0 )
        return -errno;

    // create a file descriptor for the opened file
    int os_fd = result;
    int my_fd = -1;
    try
    {
        my_fd   = m_openedFiles.registerFile( Path_t(path) ,os_fd);
        result  = 0;
    }
    catch( const std::exception& ex )
    {
        my_fd   = -1;
        result  = -ENOMEM;
        ::close(os_fd);

        std::cerr << "FuseContext::open"
                  << "\n path: " << path
                  << "\n real: " << wrapped
                  << "\n  err: " << ex.what();
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
        // open the local version of the file
        int result = ::open( wrapped.c_str(), O_RDONLY );
        if( result < 0 )
            return -errno;

        // get the file andle
        int fh = result;

        // perform the read
        result = ::pread(fh,buf,bufsize,offset);

        // close the file
        ::close(fh);

        if( result < 0 )
            return -errno;
        else
            return result;
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
    else
    {
        // otherwise open the file
        int result = ::open( wrapped.c_str(), O_WRONLY );
        if( result < 0 )
            return -errno;

        // get the file handle
        int fh = result;

        // perform the write
        result = ::pwrite(fh,buf,bufsize,offset);

        // close the file
        ::close(fh);

        // check for error
        if( result < 0 )
            return -errno;

        // increment the version
        m_backend->db().incrementVersion( Path_t(path) );

        return result;
    }
}




int FuseContext::truncate (const char *path, off_t length)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = (m_realRoot / path).string();
    int result = ::truncate( wrapped.c_str(), length  );
    if( result <  0 )
        return -errno;

    // update metadata
    m_backend->db().incrementVersion( Path_t(path) );

    return result;
}




int FuseContext::ftruncate (const char *path,
                            off_t length,
                            struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    Path_t wrapped = (m_realRoot / path).string();
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



int FuseContext::fsync (const char *path,
                        int datasync,
                        struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;

    if(fi->fh)
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if( file )
        {
            if(datasync)
                return result_or_errno( ::fdatasync(file->fd()) );
            else
                return result_or_errno( ::fsync(file->fd()) );
        }
        else
            return -EBADF;
    }
    else
        return -EBADF;
}



int FuseContext::flush (const char *path, struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    return 0;
}



int FuseContext::release (const char *path, struct fuse_file_info *fi)
{
    if(fi->fh)
        m_openedFiles.unregisterFile(fi->fh);
    else
        return -EBADF;

    return 0;
}















int FuseContext::getattr (const char *path, struct stat *out)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;

    if( m_backend->db().isSubscribed( m_relDir / path ) )
    {
        int result = ::lstat( wrapped.c_str(), out );
        if( result < 0 )
            return -errno;

        return result;
    }
    else
    {
        out->st_mode  = S_IFREG | S_IRUSR | S_IWUSR;
        out->st_nlink = 0;
        out->st_uid   = getuid();
        out->st_gid   = getgid();
        out->st_size  = 0;
        out->st_atime = 0;
        out->st_mtime = 0;
        out->st_ctime = 0;

        return 0;
    }

}




int FuseContext::fgetattr (const char *path,
                            struct stat *out,
                            struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;

    if( fi->fh )
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if(file)
        {
            int result = ::fstat(file->fd(),out);
            if( result < 0 )
                return -errno;

            return result;
        }
        else
            return -EBADF;
    }
    else
        return -EBADF;
}





int FuseContext::unlink (const char *path)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;

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
        m_backend->db().unlink( Path_t(path) );
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







int FuseContext::mkdir (const char *path, mode_t mode)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    try
    {
        m_backend->db().mknod( Path_t(path) );
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::mkdir: "
              << "\n path: " << path
              << "\n real: " << wrapped
              << "\n  err: " << ex.what()
              << "\n";
        return -EINVAL;
    }

    // create the directory
    int result = ::mkdir( wrapped.c_str(), mode );
    if( result )
        return -errno;

    return 0;
}




int FuseContext::opendir (const char *path, struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    try
    {
        int fd = m_openedFiles.registerFile( Path_t(path), -1 );
        fi->fh = fd;
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::opendir: "
          << "\n   path: " << path
          << "\n   real: " << wrapped
          << "\n  error: " << ex.what()
          << "\n";
        return -EINVAL;
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
//    std::cerr << "FuseContext::readdir" << std::endl;
//    std::cerr << "readdir: "
//                 "\n   this: " << (void*)this <<
//                 "\n   path: " << path <<
//                 "\n       : " << (m_realRoot / path) <<
//                 "\n    off: " << offset <<
//                 "\n     fh: " << fi->fh <<
//                 std::endl;

    RefPtr<FileContext> file;
    if( fi->fh )
    {
        file = m_openedFiles[fi->fh];
        if(!file)
            return -EBADF;
    }
    else
        file = FileContext::create(m_backend,Path_t(path),-1);

    m_backend->db().readdir( Path_t(path), buf, filler, offset );
    return 0;
}



int FuseContext::releasedir (const char *path,
                                struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;

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
    return 0;
}






int FuseContext::rmdir (const char *path)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;

    // first we make sure that the parent directory exists
    Path_t parent   = wrapped.parent_path();
    Path_t filename = wrapped.filename();

    if( !fs::exists(parent) )
      return -ENOENT;

    // unlink the directory holding the file contents, the meta file,
    // and the staged file
    fs::remove_all( wrapped );

    try
    {
        // remove the entry from the parent
        m_backend->db().unlink( Path_t(path) );
    }
    catch( const std::exception& ex )
    {
        std::cerr << "FuseContext::rmdir: "
              << "\n path: " << path
              << "\n real: " << wrapped
              << "\n  err: " << ex.what()
              << "\n";
    }

    return 0;
}



int FuseContext::symlink (const char *oldpath, const char *newpath)
{
    Path_t oldwrap = m_realRoot / oldpath;
    Path_t newwrap = m_realRoot / newpath;

    int result = ::symlink( oldwrap.c_str(), newwrap.c_str() );
    if( result < 0 )
        return -errno;

    return result;
}


int FuseContext::readlink (const char *path, char *buf, size_t bufsize)
{
    Path_t wrapped = m_realRoot / path;
    int result = ::readlink(  wrapped.c_str(), buf, bufsize );
    if( result < 0 )
        return -errno;

    return result;
}






int FuseContext::link (const char *oldpath, const char *newpath)
{
    Path_t oldwrap = m_realRoot / oldpath;
    Path_t newwrap = m_realRoot / newpath;

    int result = ::link( oldwrap.c_str(), newwrap.c_str() );
    if( result < 0 )
        return -errno;

    return result;
}




int FuseContext::rename (const char *oldpath, const char *newpath)
{
    namespace fs = boost::filesystem;
    Path_t oldwrap = m_realRoot / oldpath;
    Path_t newwrap = m_realRoot / newpath;

    // if the move overwrites a file then copy data, increment version, and
    // unlink the old file
    if( fs::exists( newwrap ) )
    {
        // perform the move
        int result = ::rename( oldwrap.c_str(), newwrap.c_str() );
        if( result < 0 )
            return -errno;

        m_backend->db().incrementVersion( Path_t(newpath) );
        m_backend->db().unlink( Path_t(oldpath) );

        return result;
    }
    else
    {
        int result = ::rename( oldwrap.c_str(), newwrap.c_str() );
        if( result < 0 )
            return -errno;

        m_backend->db().mknod( Path_t(newpath) );
        m_backend->db().unlink( Path_t(oldpath) );

        return result;
    }
}



int FuseContext::chmod (const char *path, mode_t mode)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = m_realRoot / path;

    // if it's not a directory
    int result = ::chmod( wrapped.c_str(), mode );
    if( result < 0 )
        return -errno;

    return result;
}



int FuseContext::chown (const char *path, uid_t owner, gid_t group)
{
    namespace fs = boost::filesystem;
    Path_t wrapped = (m_realRoot / path);

    int result = ::chown( wrapped.c_str(), owner, group);
    if( result < 0 )
        return -errno;

    return result;
}




int FuseContext::access (const char *path, int mode)
{
    Path_t wrapped = m_realRoot / path;

    int result = ::access( wrapped.c_str(), mode );
    if( result < 0 )
        return -errno;

    return result;
}








int FuseContext::lock (const char *path,
                        struct fuse_file_info *fi,
                        int cmd,
                        struct flock *fl)
{
    Path_t wrapped = m_realRoot / path;

    if( fi->fh )
    {
        RefPtr<FileContext> file = m_openedFiles[fi->fh];
        if(file)
        {
            int result = fcntl(fi->fh,cmd,fl);
            if( result < 0 )
                return -errno;
            return result;
        }
        else
            return -EBADF;
    }
    else
        return -EBADF;
}



int FuseContext::utimens (const char *path, const struct timespec tv[2])
{
    Path_t wrapped = m_realRoot / path;

    timeval times[2];
    for(int i=0; i < 2; i++)
    {
        times[i].tv_sec = tv[i].tv_sec;
        times[i].tv_usec= tv[i].tv_nsec / 1000;
    }

    int result = ::utimes( wrapped.c_str(), times );
    if( result < 0 )
        return -errno;

    return result;
}












int FuseContext::statfs (const char *path, struct statvfs *buf)
{
    Path_t wrapped = m_realRoot / path;
    int result = ::statvfs( wrapped.c_str(), buf );
    if( result < 0 )
        return -errno;

    return result;
}















int FuseContext::setxattr (const char *path,
                            const char *key,
                            const char *value,
                            size_t bufsize,
                            int flags)
{
    Path_t wrapped = m_realRoot / path;
    int result = ::setxattr( wrapped.c_str(), key, value, bufsize, flags );
    if( result < 0 )
        return -errno;

    return result;
}



int FuseContext::getxattr (const char *path,
                            const char *key,
                            char *value,
                            size_t bufsize)
{
    std::string attr(key);
    if( attr == "obfs:checkout" )
    {
        std::stringstream report;
        report << "FuseContext::getxattr : intercepted checkout hook for"
               << path <<"\n";
        std::cout << report.str();
        m_backend->checkout( m_relDir / path );
        return 0;
    }
    else if( attr == "obfs:release" )
    {
        std::stringstream report;
        report << "FuseContext::getxattr : intercepted release hook for"
               << path <<"\n";
        std::cout << report.str();
        m_backend->release( m_relDir / path );
        return 0;
    }
    else
    {
        Path_t wrapped = m_realRoot / path;
        int result = ::getxattr( wrapped.c_str(), key, value, bufsize );
        if( result < 0 )
            return -errno;

        return result;
    }

}



int FuseContext::listxattr (const char *path, char *buf, size_t bufsize)
{
    Path_t wrapped = m_realRoot / path;
    int result = ::listxattr( wrapped.c_str(), buf, bufsize );
    if( result < 0 )
        return -errno;

    return result;
}



int FuseContext::removexattr (const char *path, const char *key)
{
    Path_t wrapped = m_realRoot / path;
    int result = ::removexattr( wrapped.c_str(), key );
    if( result < 0 )
        return -errno;

    return result;
}










int FuseContext::bmap (const char *, size_t blocksize, uint64_t *idx)
{
    return -EINVAL;
}



int FuseContext::ioctl (const char *path,
                        int cmd,
                        void *arg,
                        struct fuse_file_info *fi,
                        unsigned int flags,
                        void *data)
{
    return -EINVAL;
}



int FuseContext::poll ( const char *, struct fuse_file_info *,
                      struct fuse_pollhandle *ph, unsigned *reventsp)
{
    return -EINVAL;
}




} // namespace filesystem
} // namespace openbook









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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/FuseContext.cpp
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
    Path_t wrapped = (m_realRoot / path);
    std::cerr << "FuseContext::mknod: " << path
              << "(" << (m_realRoot / path) << ")" << std::endl;

    int result = ::mknod( wrapped.c_str(), mode, dev );
    if( result )
        return -errno;

//    // create meta data file
//    fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");
//
//    try
//    {
//        MetaData metaData( metaPath );
//        metaData.load();
//        metaData.flush();
//    }
//    catch( std::exception& ex )
//    {
//        std::cerr << "Problem creating meta data: " << ex.what();
//    }

    // send message to server
//    messages::NewVersion* msg = new messages::NewVersion();
//    msg->set_job_id( m_client->nextId() );
//    msg->set_base_version(0);
//    msg->set_client_version(0);
//    msg->set_path(path);
//    msg->set_size(0);
//
//    TypedMessage tm(MSG_NEW_VERSION,msg);
//    m_comm->sendMessage(tm);

    return 0;
}




int FuseContext::create (const char *path,
                            mode_t mode,
                            struct fuse_file_info *fi)
{
    std::cerr << "FuseContext::create: "
             << "\n   path: " << path
             << "\n   real: " << (m_realRoot / path)
             << "\n   mode: " << std::hex << mode << std::dec
             << "\n";

    Path_t wrapped = m_realRoot / path;
    fi->fh = ::creat( wrapped.c_str() , mode );
    if( fi->fh < 0 )
        return -errno;

//    fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");
//
//    try
//    {
//        MetaData metaData( metaPath );
//        metaData.load();
//        metaData.flush();
//    }
//    catch( std::exception& ex )
//    {
//        std::cerr << "Problem creating meta data: " << ex.what();
//    }

    // send message to server
//    messages::NewVersion* msg = new messages::NewVersion();
//    msg->set_job_id( m_client->nextId() );
//    msg->set_base_version(0);
//    msg->set_client_version(0);
//    msg->set_path(path);
//    msg->set_size(0);

//    TypedMessage tm(MSG_NEW_VERSION,msg);
//    m_comm->sendMessage(tm);

    return 0;
}




int FuseContext::open (const char *path, struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::open: "
              << "\n    path: " << path
              << "\n    real: " << wrapped
              << "\n";

    // oen the file and get a file handle
    int fh = ::open( wrapped.c_str(), fi->flags );
    if( fh < 0 )
        return -errno;

//    // mark the file descriptor as opened
//    FileDescriptor* fd = (*m_fd)[fh];
//
//    // check to make sure we dont have too many files open already
//    if(!fd)
//    {
//        std::cerr << "Not enough file descriptors available to open " << path;
//        ::close(fh);
//        return EMFILE;
//    }
//
//    fd->open();
    fi->fh = fh;

    return 0;
}





int FuseContext::read (const char *path,
                        char *buf,
                        size_t bufsize,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;

    std::cerr << "FuseContext::read: "
              << "\n    path: " << path
              << "\n    real: " << wrapped
              << "\n    size: " << bufsize
              << "\n     off: " << offset
              << "\n      fh: " << fi->fh
              << "\n";

    if(fi->fh)
    {
        int result = ::pread(fi->fh,buf,bufsize,offset);
        if( result < 0 )
            return -errno;
        else
            return result;
    }
    else
        return -EBADF;
}



int FuseContext::write (const char *path,
                        const char *buf,
                        size_t bufsize,
                        off_t offset,
                      struct fuse_file_info *fi)
{
    std::cerr << "FuseContext::write: "
              << "\n    path: " << path
              << "\n        : " << (m_realRoot/path)
              << "\n      fh: " << fi->fh
              << "\n";

    if(fi->fh)
    {
//        // get the file descriptor
//        FileDescriptor* fd = (*m_fd)[fi->fh];
//
//        if(!fd)
//            return -EBADF;
//
//        /// lock during the write
//        pthreads::ScopedLock lock(fd->mutex());

        int result = ::pwrite(fi->fh,buf,bufsize,offset);
        if( result < 0 )
            return -errno;

//        // mark the file as changed
//        fd->flag(fd::FLAG_CHANGED,true);

        // return the result of the write
        return result;
    }
    else
    {
        Path_t wrapped = m_realRoot / path;
        int fh = ::open( wrapped.c_str(), O_WRONLY );
        if( fh < 0 )
            return -errno;
        int result = ::pwrite(fi->fh,buf,bufsize,offset);
        close(fh);
        return result_or_errno(result);
    }
}




int FuseContext::truncate (const char *path, off_t length)
{
    Path_t wrapped = (m_realRoot / path).string();
    std::cerr << "FuseContext::truncate: "
              << "\n      path: " << path
              << "\n          : " << wrapped
              << "\n    length: " << length
              << "\n";

    int result = ::truncate( wrapped.c_str(), length  );
    if( result <  0 )
        return -errno;

    // load the meta data file
//    fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");

    // send message to server
//    messages::NewVersion* msg = new messages::NewVersion();
//    msg->set_job_id( m_client->nextId() );
//    msg->set_path(path);
//    msg->set_size(length);
//
//    try
//    {
//        MetaData metaData( metaPath );
//        metaData.load();
//        // increment client version
//        metaData.set_clientVersion( metaData.clientVersion() + 1 );
//        msg->set_base_version(metaData.baseVersion());
//        msg->set_client_version(metaData.clientVersion());
//        metaData.flush();
//
//        // send the message
//        TypedMessage tm(MSG_NEW_VERSION,msg);
//        m_comm->sendMessage(tm);
//    }
//    catch( std::exception& ex )
//    {
//        std::cerr << "Problem creating updating meta data: "
//                  << ex.what() << std::endl;
//        delete msg;
//    }

    return result;
}




int FuseContext::ftruncate (const char *path,
                            off_t length,
                            struct fuse_file_info *fi)
{
    std::cerr << "FuseContext::ftruncate: "
              << "\n   path: " << path
              << "\n       : " << (m_realRoot / path)
              << "\n    len: " << length
              << "\n     fh: " << fi->fh
              << "\n";

    if(fi->fh)
    {
//        // get the file descriptor
//        FileDescriptor* fd = (*m_fd)[fi->fh];
//
//        if(!fd)
//            return -EBADF;
//
//        /// lock during the write
//        pthreads::ScopedLock lock(fd->mutex());

        int result = ::ftruncate(fi->fh, length);
        if( result < 0 )
            return -errno;

//        // mark the file as changed
//        fd->flag(fd::FLAG_CHANGED,true);

        // return the result of the write
        return result;
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
//        // get the file descriptor
//        FileDescriptor* fd = (*m_fd)[fi->fh];
//
//        if(!fd)
//            return -EBADF;
//
//        /// lock during the write
//        pthreads::ScopedLock lock(fd->mutex());
//
//        // if the file has changed then send update to the server
//        if( fd->flag(fd::FLAG_CHANGED) )
//        {
//            fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");

//            messages::NewVersion* msg = new messages::NewVersion();
//            msg->set_job_id( m_client->nextId() );
//            msg->set_path(path);
//
//            try
//            {
//                // stat the file to get the size
//                struct stat fs;
//                int result = ::fstat(fi->fh,&fs);
//                if( result < 0 )
//                    ex()() << "Failed to stat file " << path
//                           << " after changes " ;
//                msg->set_size(fs.st_size);
//
//                MetaData metaData( metaPath );
//                metaData.load();
//                // increment client version
//                metaData.set_clientVersion( metaData.clientVersion() + 1 );
//                msg->set_base_version(metaData.baseVersion());
//                msg->set_client_version(metaData.clientVersion());
//
//                std::cout << "Sending new version message for " << path
//                          << "\n     base: " << metaData.baseVersion()
//                          << "\n   client: " << metaData.clientVersion()
//                          << std::endl;
//
//                metaData.flush();
//
//                // send the message
//                TypedMessage tm(MSG_NEW_VERSION,msg);
//                m_comm->sendMessage(tm);
//            }
//            catch( std::exception& ex )
//            {
//                std::cerr << "Problem updating meta data: "
//                          << ex.what() << std::endl;
//                delete msg;
//            }
//        }

//        // close the file and mark it as closed mark the fd as closed
//        fd->flag(fd::FLAG_OPENED, false);
        int result = ::close(fi->fh);
        if( result < 0 )
            return -errno;

        // return the result of the write
        return result;
    }
    else
        return -EBADF;
}















int FuseContext::getattr (const char *path, struct stat *out)
{
    Path_t wrapped = m_realRoot / path;

    std::cerr << "FuseContext::getattr: "
              << "\n       this: " << (void*) this
              << "\n    request: " << path
              << "\n translated: " << wrapped
              << "\n";

    return ::lstat( wrapped.c_str(), out ) ? -errno : 0;
}



int FuseContext::readlink (const char *path, char *buf, size_t bufsize)
{
    Path_t wrapped = m_realRoot / path;

    std::cerr << "FuseContext::readlink: "
              << path
              << "(" << wrapped << ")\n";

    return ::readlink(  wrapped.c_str(), buf, bufsize ) ? -errno : 0;
}









int FuseContext::mkdir (const char *path, mode_t mode)
{
    Path_t wrapped = m_realRoot / path;

    std::cerr << "FuseContext::mkdir: "
              << path
              << "(" << wrapped << ")\n";

    return ::mkdir(  wrapped.c_str() , mode|S_IFDIR ) ? -errno : 0;
}



int FuseContext::unlink (const char *path)
{
    Path_t wrapped = m_realRoot / path;
    std::cerr << "FuseContext::unlink: "
              << path
              << "(" << wrapped << ")\n";

    int result = ::unlink( wrapped.c_str() );
    if( result )
        return -errno;

//    fs::path metaFile = m_realRoot / ( std::string(path) + ".obfsmeta" );
//    result = ::unlink ( metaFile.c_str() );

    return 0;
}



int FuseContext::rmdir (const char *path)
{
    Path_t wrapped = m_realRoot / path;

    std::cerr << "rmdir: "
              << path
              << "(" << wrapped << ")\n";

    return result_or_errno( ::rmdir(  wrapped.c_str()  ) );
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
    std::cerr << "FuseContext::opendir: "
              << "\n   path: " << path
              << "\n       : " << wrapped
              << "\n";

    DIR* dir = ::opendir( wrapped.c_str() );

    if(dir)
    {
        fi->fh = (uint64_t)dir;
        return 0;
    }
    else
        return -errno;
}



int FuseContext::readdir (const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    std::cerr << "FuseContext::readdir" << std::endl;
    std::cerr << "readdir: "
                 "\n   this: " << (void*)this <<
                 "\n   path: " << path <<
                 "\n       : " << (m_realRoot / path) <<
                 "\n    off: " << offset <<
                 "\n     fh: " << fi->fh <<
                 std::endl;


    DIR* dp = (DIR*) fi->fh;

    struct dirent *de;
    int retstat = 0;

    // Every directory contains at least two entries: . and ..  If my
    // first call to the system readdir() returns NULL I've got an
    // error; near as I can tell, that's the only condition under
    // which I can get an error from readdir()
    de = ::readdir(dp);
    if (de == 0)
        return -errno;

    // This will copy the entire directory into the buffer.  The loop exits
    // when either the system readdir() returns NULL, or filler()
    // returns something non-zero.  The first case just means I've
    // read the whole directory; the second means the buffer is full.
    do
    {
        if (filler(buf, de->d_name, NULL, 0) != 0)
            return -ENOMEM;
    } while ((de = ::readdir(dp)) != NULL);

    return retstat;
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
    {
        DIR* dp = (DIR*) fi->fh;
        return result_or_errno( ::closedir(dp) );
    }
    else
        return -EBADF;
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








int FuseContext::fgetattr (const char *path,
                            struct stat *out,
                            struct fuse_file_info *fi)
{
    Path_t wrapped = m_realRoot / path;

    std::cerr << "FuseContext::fgetattr: "
              << "\n       this: " << (void*) this
              << "\n    request: " << path
              << "\n translated: " << wrapped
              << "\n";

    return result_or_errno( ::fstat( fi->fh, out ) );
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









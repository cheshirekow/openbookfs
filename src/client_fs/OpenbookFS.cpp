/*
 * OpenbookFS.cpp
 *
 *  Created on: Jun 7, 2012
 *      Author: josh
 */

#include "OpenbookFS.h"


#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include <dirent.h>
#include <sys/time.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

namespace   openbook {
namespace filesystem {


int OpenbookFS::result_or_errno(int result)
{
    if(result < 0)
        return -errno;
    else
        return result;
}



OpenbookFS::OpenbookFS(boost::filesystem::path dataDir)
{
    m_dataDir = dataDir;

}




OpenbookFS::~OpenbookFS()
{
}




int OpenbookFS::getattr (const char *path, struct stat *out)
{
    std::string wrapped = (m_dataDir / path).string();

    std::cerr << "getattr: "
                << "\n       this: " << (void*) this
                << "\n    request: " << path
                << "\n translated: "
                    << "(" << wrapped << ")" << std::endl;

    int result = ::lstat( wrapped.c_str(), out );
    if( result )
        std::cerr << "Problem!!" << std::endl;
    else
        std::cerr << "No problem" << std::endl;
    return result ? -errno : 0;
}



int OpenbookFS::readlink (const char *path, char *buf, size_t bufsize)
{
    std::string wrapped = (m_dataDir / path).string();

    std::cerr << "readlink: " << path
                << "(" << (m_dataDir / path) << ")" << std::endl;

    return ::readlink(  wrapped.c_str(), buf, bufsize ) ? -errno : 0;
}





int OpenbookFS::mknod (const char *path, mode_t mode, dev_t dev)
{
    std::string wrapped = (m_dataDir / path).string();
    std::cerr << "mknod: " << path
                << " (" << (m_dataDir / path) << ")" << std::endl;

    return ::mknod( wrapped.c_str(), mode, dev ) ? -errno : 0;
}



int OpenbookFS::mkdir (const char *pathname, mode_t mode)
{
    std::string wrapped = (m_dataDir / pathname).string();

    std::cerr << "mkdir: " << pathname
                << "(" << wrapped << ")" << std::endl;
    return ::mkdir(  wrapped.c_str() , mode|S_IFDIR ) ? -errno : 0;
}



int OpenbookFS::unlink (const char *pathname)
{
    std::string wrapped = (m_dataDir / pathname).string();
    std::cerr << "unlink: " << pathname
                << "(" << wrapped << ")" << std::endl;

    return result_or_errno( ::unlink(  wrapped.c_str()  ) );
}



int OpenbookFS::rmdir (const char *pathname)
{
    std::string wrapped = (m_dataDir / pathname).string();

    std::cerr << "rmdir: " << pathname
                << "(" << wrapped << ")" << std::endl;

    return result_or_errno( ::rmdir(  wrapped.c_str()  ) );
}



int OpenbookFS::symlink (const char *oldpath, const char *newpath)
{
    std::string oldwrap = (m_dataDir / oldpath).string();
    std::string newwrap = (m_dataDir / oldpath).string();

    std::cerr << "simlink: "
                  "\n   old: " << oldpath <<
                  "\n      : " << oldwrap <<
                  "\n   new: " << newpath <<
                  "\n      : " << newwrap <<
                  std::endl;

    return result_or_errno( ::symlink( oldwrap.c_str(), newwrap.c_str() ) );
}



int OpenbookFS::rename (const char *oldpath, const char *newpath)
{
    std::string oldwrap = (m_dataDir / oldpath).string();
    std::string newwrap = (m_dataDir / oldpath).string();

    std::cerr << "rename: "
                  "\n   old: " << oldpath <<
                  "\n      : " << oldwrap <<
                  "\n   new: " << newpath <<
                  "\n      : " << newwrap <<
                  std::endl;

    return result_or_errno( ::rename( oldwrap.c_str(), newwrap.c_str() ) );
}



int OpenbookFS::link (const char *oldpath, const char *newpath)
{
    std::string oldwrap = (m_dataDir / oldpath).string();
    std::string newwrap = (m_dataDir / oldpath).string();

    std::cerr << "link: "
                  "\n   old: " << oldpath <<
                  "\n      : " << oldwrap <<
                  "\n   new: " << newpath <<
                  "\n      : " << newwrap <<
                  std::endl;

    return result_or_errno( ::link( oldwrap.c_str(), newwrap.c_str() ) );
}



int OpenbookFS::chmod (const char *path, mode_t mode)
{
    std::string wrapped = (m_dataDir / path).string();
    std::cerr << "chmod: "
              "\n    path: " << path    <<
              "\n        : " << wrapped <<
              "\n    mode: " << mode    <<
              std::endl;

    return result_or_errno( ::chmod( wrapped.c_str(), mode ) );
}



int OpenbookFS::chown (const char *path, uid_t owner, gid_t group)
{
    std::string wrapped = (m_dataDir / path).string();
    std::cerr << "chown: "
              "\n    path: " << path                <<
              "\n        : " << (m_dataDir/path)    <<
              "\n     uid: " << owner               <<
              "\n     gid: " << group               <<
              std::endl;

    return  result_or_errno( ::chown( wrapped.c_str(), owner, group ) );
}



int OpenbookFS::truncate (const char *path, off_t length)
{
    std::string wrapped = (m_dataDir / path).string();
    std::cerr << "truncate: "
              "\n      path: " << path    <<
              "\n          : " << wrapped <<
              "\n    length: " << length  <<
              std::endl;

    return  result_or_errno( ::truncate( wrapped.c_str(), length  ) );
}



//int OpenbookFS::utime (const char *, struct utimbuf *)
//{
//
//}



int OpenbookFS::open (const char *path, struct fuse_file_info *fi)
{
    std::string wrapped = (m_dataDir / path).string();
    std::cerr << "open: "
              "\n    path: " << path    <<
              "\n        : " << wrapped <<
              std::endl;

    int fh = ::open( wrapped.c_str(), fi->flags );
    fi->fh = fh;

    return result_or_errno(( fh > 0 ) ? 0 : fh);
}



int OpenbookFS::read (const char *pathname,
                        char *buf,
                        size_t bufsize,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    std::string wrapped = (m_dataDir / pathname).string();

    std::cerr << "read: "
              "\n    path: " << pathname  <<
              "\n        : " << wrapped   <<
              "\n    size: " << bufsize   <<
              "\n     off: " << offset    <<
              "\n      fh: " << fi->fh    <<
              std::endl;

    if(fi->fh)
    {
        int result = result_or_errno( ::pread(fi->fh,buf,bufsize,offset) );
        return result;
    }
    else
        return -EBADF;
}



int OpenbookFS::write (const char *pathname,
                        const char *buf,
                        size_t bufsize,
                        off_t offset,
                      struct fuse_file_info *fi)
{
    std::cerr << "write: "
              "\n    path: " << pathname                <<
              "\n        : " << (m_dataDir/pathname)    <<
              "\n      fh: " << fi->fh                  <<
              std::endl;

    if(fi->fh)
        return result_or_errno( ::pwrite(fi->fh,buf,bufsize,offset) );
    else
        return -EBADF;
}



int OpenbookFS::statfs (const char *path, struct statvfs *buf)
{
    std::string wrapped = (m_dataDir / path).string();
    std::cerr << "statfs: "
                "\n   path: " << path <<
                "\n       : " << (m_dataDir / path)
                << std::endl;

    return result_or_errno( ::statvfs( wrapped.c_str(), buf ) );
}



int OpenbookFS::flush (const char *path, struct fuse_file_info *fi)
{
    std::cerr << "flush: "
                 "\n   path: " << path <<
                 "\n       : " << (m_dataDir / path) <<
                 "\n     fh: " << fi->fh <<
                 std::endl;


    return 0;
}



int OpenbookFS::release (const char *path, struct fuse_file_info *fi)
{
    std::cerr << "release: "
                 "\n   path: " << path <<
                 "\n       : " << (m_dataDir / path) <<
                 "\n     fh: " << fi->fh;

    if(fi->fh)
    {
        int result = result_or_errno( ::close(fi->fh) );
        return result;
    }
    else
        return -EBADF;
}



int OpenbookFS::fsync (const char *path,
                        int datasync,
                        struct fuse_file_info *fi)
{
    std::cerr << "fsync: "
                 "\n   path: " << path <<
                 "\n       : " << (m_dataDir / path) <<
                 "\n     ds: " << datasync <<
                 "\n     fh: " << fi->fh;

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



int OpenbookFS::setxattr (const char *path,
                            const char *key,
                            const char *value,
                            size_t bufsize,
                            int unknown)
{
    std::cerr << "setxattr" << std::endl;
    return 0;
}



int OpenbookFS::getxattr (const char *path,
                            const char *key,
                            char *value,
                            size_t bufsize)
{
    std::cerr << "getxattr" << std::endl;
    return 0;
}



int OpenbookFS::listxattr (const char *path, char *buf, size_t bufsize)
{
    std::cerr << "listxattr" << std::endl;
    return 0;
}



int OpenbookFS::removexattr (const char *path, const char *key)
{
    std::cerr << "removexattr" << std::endl;
    return 0;
}



int OpenbookFS::opendir (const char *path, struct fuse_file_info *fi)
{
    std::cerr << "opendir" << std::endl;
    std::cerr << "opendir: "
                 "\n   path: " << path <<
                 "\n       : " << (m_dataDir / path) <<
                 std::endl;

    std::string wrapped = (m_dataDir / path).string();
    DIR* dir = ::opendir( wrapped.c_str() );

    if(dir)
    {
        fi->fh = (uint64_t)dir;
        return 0;
    }
    else
        return -errno;
}



int OpenbookFS::readdir (const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    std::cerr << "readdir" << std::endl;
    std::cerr << "readdir: "
                 "\n   this: " << (void*)this <<
                 "\n   path: " << path <<
                 "\n       : " << (m_dataDir / path) <<
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



int OpenbookFS::releasedir (const char *path,
                                struct fuse_file_info *fi)
{
    std::cerr << "releasedir: "
             "\n   path: " << path <<
             "\n       : " << (m_dataDir / path) <<
             "\n     fh: " << fi->fh <<
             std::endl;

    if(fi->fh)
    {
        DIR* dp = (DIR*) fi->fh;
        return result_or_errno( ::closedir(dp) );
    }
    else
        return -EBADF;
}



int OpenbookFS::fsyncdir (const char *path,
                            int datasync,
                            struct fuse_file_info *fi)
{
    std::cerr << "syncdir: "
             "\n   path: " << path <<
             "\n       : " << (m_dataDir / path) <<
             "\n     ds: " << datasync <<
             "\n     fh: " << fi->fh <<
             std::endl;
    return 0;
}



void *OpenbookFS::init (struct fuse_conn_info *conn)
{
    std::cerr << "init: " << std::endl;
    return this;
}



void OpenbookFS::destroy (void *)
{
    std::cerr << "destroy: " << std::endl;
    return;
}



int OpenbookFS::access (const char *pathname, int mode)
{
    std::cerr << "access: "
             "\n   path: " << pathname <<
             "\n       : " << (m_dataDir / pathname) <<
             "\n   mode: " << mode <<
             std::endl;

    std::string wrapped = (m_dataDir / pathname).string();
    return result_or_errno( ::access( wrapped.c_str(), mode ) );
}



int OpenbookFS::create (const char *pathname,
                            mode_t mode,
                            struct fuse_file_info *fi)
{
    std::cerr << "create: "
             "\n   path: " << pathname <<
             "\n       : " << (m_dataDir / pathname) <<
             "\n   mode: " << mode <<
             std::endl;

    std::string wrapped = (m_dataDir / pathname).string();
    fi->fh = ::creat( wrapped.c_str() , mode );
    if( fi->fh > 0 )
        return 0;
    else
        return -errno;
}



int OpenbookFS::ftruncate (const char *path,
                            off_t length,
                            struct fuse_file_info *fi)
{
    std::cerr << "ftruncate: "
             "\n   path: " << path <<
             "\n       : " << (m_dataDir / path) <<
             "\n    len: " << length <<
             "\n     fh: " << fi->fh <<
             std::endl;

    if(fi->fh)
    {
        return result_or_errno(
                ::ftruncate(fi->fh, length)
                 );
    }
    else
        return -EBADF;
}



int OpenbookFS::fgetattr (const char *path,
                            struct stat *out,
                            struct fuse_file_info *fi)
{
    std::string wrapped = (m_dataDir / path).string();

    std::cerr << "fgetattr: "
                << "\n       this: " << (void*) this
                << "\n    request: " << path
                << "\n translated: "
                    << "(" << wrapped << ")" << std::endl;

    return result_or_errno( ::fstat( fi->fh, out ) );
}



int OpenbookFS::lock (const char *path,
                        struct fuse_file_info *fi,
                        int cmd,
                        struct flock *fl)
{
    std::cerr << "lock: "
             "\n   path: " << path <<
             "\n       : " << (m_dataDir / path) <<
             "\n       : " << cmd <<
             "\n     fh: " << fi->fh <<
             std::endl;

    if( fi->fh )
    {
        return result_or_errno(
                fcntl(fi->fh,cmd,fl)
                );
    }
    else
        return -EBADF;
}



int OpenbookFS::utimens (const char *path, const struct timespec tv[2])
{
    std::cerr << "utimens: "
             "\n   path: " << path <<
             "\n       : " << (m_dataDir / path) <<
             "\n     t1: " << tv[0].tv_sec << " : " << tv[0].tv_nsec <<
             "\n     t2: " << tv[1].tv_sec << " : " << tv[1].tv_nsec <<
             std::endl;

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



int OpenbookFS::bmap (const char *, size_t blocksize, uint64_t *idx)
{
    std::cerr << "bmap" << std::endl;
    return 0;
}



int OpenbookFS::ioctl (const char *path,
                        int cmd,
                        void *arg,
                        struct fuse_file_info *fi,
                        unsigned int flags,
                        void *data)
{
    std::cerr << "ioctl" << std::endl;
    return 0;
}



int OpenbookFS::poll ( const char *, struct fuse_file_info *,
                      struct fuse_pollhandle *ph, unsigned *reventsp)
{
    std::cerr << "poll" << std::endl;
    return 0;
}




} // namespace filesystem
} // namespace openbook


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
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "MetaFile.h"

namespace   openbook {
namespace filesystem {
namespace     client {



int OpenbookFS::result_or_errno(int result)
{
    if(result < 0)
        return -errno;
    else
        return result;
}





OpenbookFS::OpenbookFS(
        Client*              client,
        ServerHandler*       comm,
        FileDescriptorArray* fd)
{
    m_client   = client;
    m_comm     = comm;
    m_fd       = fd;
    m_dataDir  = client->dataDir();
    m_realRoot = m_dataDir / "real_root";
}




OpenbookFS::~OpenbookFS()
{
}




int OpenbookFS::mknod (const char *path, mode_t mode, dev_t dev)
{
    namespace fs = boost::filesystem;
    fs::path wrapped = (m_realRoot / path);
    std::cerr << "mknod: " << path
                << " (" << (m_realRoot / path) << ")" << std::endl;

    int result = ::mknod( wrapped.c_str(), mode, dev );
    if( result )
        return -errno;

    // create meta data file
    fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");

    try
    {
        MetaData metaData( metaPath );
        metaData.load();
        metaData.flush();
    }
    catch( std::exception& ex )
    {
        std::cerr << "Problem creating meta data: " << ex.what();
    }

    // send message to server
    messages::NewVersion* msg = new messages::NewVersion();
    msg->set_job_id( m_client->nextId() );
    msg->set_base_version(0);
    msg->set_client_version(0);
    msg->set_path(path);
    msg->set_size(0);

    TypedMessage tm(MSG_NEW_VERSION,msg);
    m_comm->sendMessage(tm);

    return 0;
}




int OpenbookFS::create (const char *path,
                            mode_t mode,
                            struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    std::cerr << "create: "
             "\n   path: " << path <<
             "\n       : " << (m_realRoot / path) <<
             "\n   mode: " << mode <<
             std::endl;

    fs::path wrapped = (m_realRoot / path).string();
    fi->fh = ::creat( wrapped.c_str() , mode );
    if( fi->fh < 0 )
        return -errno;

    fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");

    try
    {
        MetaData metaData( metaPath );
        metaData.load();
        metaData.flush();
    }
    catch( std::exception& ex )
    {
        std::cerr << "Problem creating meta data: " << ex.what();
    }

    // send message to server
    messages::NewVersion* msg = new messages::NewVersion();
    msg->set_job_id( m_client->nextId() );
    msg->set_base_version(0);
    msg->set_client_version(0);
    msg->set_path(path);
    msg->set_size(0);

    TypedMessage tm(MSG_NEW_VERSION,msg);
    m_comm->sendMessage(tm);

    return 0;
}




int OpenbookFS::open (const char *path, struct fuse_file_info *fi)
{
    std::string wrapped = (m_realRoot / path).string();
    std::cerr << "open: "
              "\n    path: " << path    <<
              "\n        : " << wrapped <<
              std::endl;

    // oen the file and get a file handle
    int fh = ::open( wrapped.c_str(), fi->flags );
    if( fh < 0 )
        return -errno;

    // mark the file descriptor as opened
    FileDescriptor* fd = (*m_fd)[fh];

    // check to make sure we dont have too many files open already
    if(!fd)
    {
        std::cerr << "Not enough file descriptors available to open " << path;
        ::close(fh);
        return EMFILE;
    }

    fd->open();
    fi->fh = fh;

    return 0;
}





int OpenbookFS::read (const char *path,
                        char *buf,
                        size_t bufsize,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    std::string wrapped = (m_realRoot / path).string();

    std::cerr << "read: "
              "\n    path: " << path  <<
              "\n        : " << wrapped   <<
              "\n    size: " << bufsize   <<
              "\n     off: " << offset    <<
              "\n      fh: " << fi->fh    <<
              std::endl;

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



int OpenbookFS::write (const char *path,
                        const char *buf,
                        size_t bufsize,
                        off_t offset,
                      struct fuse_file_info *fi)
{
    std::cerr << "write: "
              "\n    path: " << path                <<
              "\n        : " << (m_realRoot/path)    <<
              "\n      fh: " << fi->fh                  <<
              std::endl;

    if(fi->fh)
    {
        // get the file descriptor
        FileDescriptor* fd = (*m_fd)[fi->fh];

        if(!fd)
            return -EBADF;

        /// lock during the write
        pthreads::ScopedLock lock(fd->mutex());

        int result = ::pwrite(fi->fh,buf,bufsize,offset);
        if( result < 0 )
            return -errno;

        // mark the file as changed
        fd->flag(fd::FLAG_CHANGED,true);

        // return the result of the write
        return result;
    }
    else
        return -EBADF;
}




int OpenbookFS::truncate (const char *path, off_t length)
{
    namespace fs = boost::filesystem;

    std::string wrapped = (m_realRoot / path).string();
    std::cerr << "truncate: "
              "\n      path: " << path    <<
              "\n          : " << wrapped <<
              "\n    length: " << length  <<
              std::endl;

    int result = ::truncate( wrapped.c_str(), length  );
    if( result <  0 )
        return -errno;

    // load the meta data file
    fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");

    // send message to server
    messages::NewVersion* msg = new messages::NewVersion();
    msg->set_job_id( m_client->nextId() );
    msg->set_path(path);
    msg->set_size(length);

    try
    {
        MetaData metaData( metaPath );
        metaData.load();
        // increment client version
        metaData.set_clientVersion( metaData.clientVersion() + 1 );
        msg->set_base_version(metaData.baseVersion());
        msg->set_client_version(metaData.clientVersion());
        metaData.flush();

        // send the message
        TypedMessage tm(MSG_NEW_VERSION,msg);
        m_comm->sendMessage(tm);
    }
    catch( std::exception& ex )
    {
        std::cerr << "Problem creating updating meta data: "
                  << ex.what() << std::endl;
        delete msg;
    }

    return result;
}




int OpenbookFS::ftruncate (const char *path,
                            off_t length,
                            struct fuse_file_info *fi)
{
    std::cerr << "ftruncate: "
             "\n   path: " << path <<
             "\n       : " << (m_realRoot / path) <<
             "\n    len: " << length <<
             "\n     fh: " << fi->fh <<
             std::endl;

    if(fi->fh)
    {
        // get the file descriptor
        FileDescriptor* fd = (*m_fd)[fi->fh];

        if(!fd)
            return -EBADF;

        /// lock during the write
        pthreads::ScopedLock lock(fd->mutex());

        int result = ::ftruncate(fi->fh, length);
        if( result < 0 )
            return -errno;

        // mark the file as changed
        fd->flag(fd::FLAG_CHANGED,true);

        // return the result of the write
        return result;
    }
    else
        return -EBADF;
}





int OpenbookFS::release (const char *path, struct fuse_file_info *fi)
{
    namespace fs = boost::filesystem;

    std::cerr << "release: "
                 "\n   path: " << path <<
                 "\n       : " << (m_realRoot / path) <<
                 "\n     fh: " << fi->fh;

    if(fi->fh)
    {
        // get the file descriptor
        FileDescriptor* fd = (*m_fd)[fi->fh];

        if(!fd)
            return -EBADF;

        /// lock during the write
        pthreads::ScopedLock lock(fd->mutex());

        // if the file has changed then send update to the server
        if( fd->flag(fd::FLAG_CHANGED) )
        {
            fs::path metaPath = m_realRoot / (std::string(path) + ".obfsmeta");

            messages::NewVersion* msg = new messages::NewVersion();
            msg->set_job_id( m_client->nextId() );
            msg->set_path(path);

            try
            {
                // stat the file to get the size
                struct stat fs;
                int result = ::fstat(fi->fh,&fs);
                if( result < 0 )
                    ex()() << "Failed to stat file " << path
                           << " after changes " ;
                msg->set_size(fs.st_size);

                MetaData metaData( metaPath );
                metaData.load();
                // increment client version
                metaData.set_clientVersion( metaData.clientVersion() + 1 );
                msg->set_base_version(metaData.baseVersion());
                msg->set_client_version(metaData.clientVersion());

                std::cout << "Sending new version message for " << path
                          << "\n     base: " << metaData.baseVersion()
                          << "\n   client: " << metaData.clientVersion()
                          << std::endl;

                metaData.flush();

                // send the message
                TypedMessage tm(MSG_NEW_VERSION,msg);
                m_comm->sendMessage(tm);
            }
            catch( std::exception& ex )
            {
                std::cerr << "Problem updating meta data: "
                          << ex.what() << std::endl;
                delete msg;
            }
        }

        // close the file and mark it as closed mark the fd as closed
        fd->flag(fd::FLAG_OPENED, false);
        int result = ::close(fi->fh);
        if( result < 0 )
            return -errno;

        // return the result of the write
        return result;
    }
    else
        return -EBADF;
}















int OpenbookFS::getattr (const char *path, struct stat *out)
{
    std::string wrapped = (m_realRoot / path).string();

    std::cerr << "getattr: "
                << "\n       this: " << (void*) this
                << "\n    request: " << path
                << "\n translated: "
                    << "(" << wrapped << ")" << std::endl;

    return ::lstat( wrapped.c_str(), out ) ? -errno : 0;
}



int OpenbookFS::readlink (const char *path, char *buf, size_t bufsize)
{
    std::string wrapped = (m_realRoot / path).string();

    std::cerr << "readlink: " << path
                << "(" << (m_realRoot / path) << ")" << std::endl;

    return ::readlink(  wrapped.c_str(), buf, bufsize ) ? -errno : 0;
}









int OpenbookFS::mkdir (const char *path, mode_t mode)
{
    std::string wrapped = (m_realRoot / path).string();

    std::cerr << "mkdir: " << path
                << "(" << wrapped << ")" << std::endl;
    return ::mkdir(  wrapped.c_str() , mode|S_IFDIR ) ? -errno : 0;
}



int OpenbookFS::unlink (const char *path)
{
    namespace fs = boost::filesystem;
    fs::path wrapped = (m_realRoot / path);
    std::cerr << "unlink: " << path
                << "(" << wrapped << ")" << std::endl;

    int result = ::unlink( wrapped.c_str() );
    if( result )
        return -errno;

    fs::path metaFile = m_realRoot / ( std::string(path) + ".obfsmeta" );
    result = ::unlink ( metaFile.c_str() );

    return 0;
}



int OpenbookFS::rmdir (const char *path)
{
    std::string wrapped = (m_realRoot / path).string();

    std::cerr << "rmdir: " << path
                << "(" << wrapped << ")" << std::endl;

    return result_or_errno( ::rmdir(  wrapped.c_str()  ) );
}



int OpenbookFS::symlink (const char *oldpath, const char *newpath)
{
    std::string oldwrap = (m_realRoot / oldpath).string();
    std::string newwrap = (m_realRoot / oldpath).string();

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
    std::string oldwrap = (m_realRoot / oldpath).string();
    std::string newwrap = (m_realRoot / oldpath).string();

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
    std::string oldwrap = (m_realRoot / oldpath).string();
    std::string newwrap = (m_realRoot / oldpath).string();

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
    std::string wrapped = (m_realRoot / path).string();
    std::cerr << "chmod: "
              "\n    path: " << path    <<
              "\n        : " << wrapped <<
              "\n    mode: " << mode    <<
              std::endl;

    return result_or_errno( ::chmod( wrapped.c_str(), mode ) );
}



int OpenbookFS::chown (const char *path, uid_t owner, gid_t group)
{
    std::string wrapped = (m_realRoot / path).string();
    std::cerr << "chown: "
              "\n    path: " << path                <<
              "\n        : " << (m_realRoot/path)    <<
              "\n     uid: " << owner               <<
              "\n     gid: " << group               <<
              std::endl;

    return  result_or_errno( ::chown( wrapped.c_str(), owner, group ) );
}







//int OpenbookFS::utime (const char *, struct utimbuf *)
//{
//
//}









int OpenbookFS::statfs (const char *path, struct statvfs *buf)
{
    std::string wrapped = (m_realRoot / path).string();
    std::cerr << "statfs: "
                "\n   path: " << path <<
                "\n       : " << (m_realRoot / path)
                << std::endl;

    return result_or_errno( ::statvfs( wrapped.c_str(), buf ) );
}



int OpenbookFS::flush (const char *path, struct fuse_file_info *fi)
{
    std::cerr << "flush: "
                 "\n   path: " << path <<
                 "\n       : " << (m_realRoot / path) <<
                 "\n     fh: " << fi->fh <<
                 std::endl;


    return 0;
}







int OpenbookFS::fsync (const char *path,
                        int datasync,
                        struct fuse_file_info *fi)
{
    std::cerr << "fsync: "
                 "\n   path: " << path <<
                 "\n       : " << (m_realRoot / path) <<
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
                 "\n       : " << (m_realRoot / path) <<
                 std::endl;

    std::string wrapped = (m_realRoot / path).string();
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



int OpenbookFS::releasedir (const char *path,
                                struct fuse_file_info *fi)
{
    std::cerr << "releasedir: "
             "\n   path: " << path <<
             "\n       : " << (m_realRoot / path) <<
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
             "\n       : " << (m_realRoot / path) <<
             "\n     ds: " << datasync <<
             "\n     fh: " << fi->fh <<
             std::endl;
    return 0;
}





int OpenbookFS::access (const char *path, int mode)
{
    std::cerr << "access: "
             "\n   path: " << path <<
             "\n       : " << (m_realRoot / path) <<
             "\n   mode: " << mode <<
             std::endl;

    std::string wrapped = (m_realRoot / path).string();
    return result_or_errno( ::access( wrapped.c_str(), mode ) );
}








int OpenbookFS::fgetattr (const char *path,
                            struct stat *out,
                            struct fuse_file_info *fi)
{
    std::string wrapped = (m_realRoot / path).string();

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
             "\n       : " << (m_realRoot / path) <<
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
             "\n       : " << (m_realRoot / path) <<
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




} // namespace client
} // namespace filesystem
} // namespace openbook


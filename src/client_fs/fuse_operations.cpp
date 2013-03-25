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
 *  @file   src/client_fs/fuse_operations.cpp
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#include <cstring>

#include "OpenbookFS.h"
#include "fuse_operations.h"


namespace   openbook {
namespace filesystem {
namespace     client {



void setFuseOps( fuse_operations& fuse_ops )
{
    memset(&fuse_ops,0,sizeof(fuse_operations));

    fuse_ops.getattr     = fuse_ops::getattr;
    fuse_ops.readlink    = fuse_ops::readlink;
    fuse_ops.getdir      = NULL;
    fuse_ops.mknod       = fuse_ops::mknod;
    fuse_ops.mkdir       = fuse_ops::mkdir;
    fuse_ops.unlink      = fuse_ops::unlink;
    fuse_ops.rmdir       = fuse_ops::rmdir;
    fuse_ops.symlink     = fuse_ops::symlink;
    fuse_ops.rename      = fuse_ops::rename;
    fuse_ops.link        = fuse_ops::link;
    fuse_ops.chmod       = fuse_ops::chmod;
    fuse_ops.chown       = fuse_ops::chown;
    fuse_ops.truncate    = fuse_ops::truncate;
    fuse_ops.utime       = NULL;
    fuse_ops.open        = fuse_ops::open;
    fuse_ops.read        = fuse_ops::read;
    fuse_ops.write       = fuse_ops::write;
    fuse_ops.statfs      = fuse_ops::statfs;
    fuse_ops.flush       = fuse_ops::flush;
    fuse_ops.release     = fuse_ops::release;
    fuse_ops.fsync       = fuse_ops::fsync;
#ifdef HAVE_SETXATTR
    fuse_ops.setxattr    = fuse_ops::setxattr;
    fuse_ops.getxattr    = fuse_ops::getxattr;
    fuse_ops.listxattr   = fuse_ops::listxattr;
    fuse_ops.removexattr = fuse_ops::removexattr;
#endif
    fuse_ops.opendir     = fuse_ops::opendir;
    fuse_ops.readdir     = fuse_ops::readdir;
    fuse_ops.releasedir  = fuse_ops::releasedir;
    fuse_ops.fsyncdir    = fuse_ops::fsyncdir;
    fuse_ops.init        = fuse_ops::init;
    fuse_ops.destroy     = fuse_ops::destroy;
    fuse_ops.access      = fuse_ops::access;
    fuse_ops.create      = fuse_ops::create;
    fuse_ops.ftruncate   = fuse_ops::ftruncate;
    fuse_ops.fgetattr    = fuse_ops::fgetattr;
    fuse_ops.lock        = fuse_ops::lock;
    fuse_ops.utimens     = fuse_ops::utimens;
    fuse_ops.bmap        = NULL;
    fuse_ops.ioctl       = fuse_ops::ioctl;
    fuse_ops.poll        = fuse_ops::poll;
}


} // namespace client
} // namespace filesystem
} // namespace openbook






namespace   openbook {
namespace filesystem {
namespace     client {

namespace   fuse_ops {



int getattr (const char *path, struct stat *out)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->getattr(path,out);
}



int readlink (const char *path, char *buf, size_t bufsize)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->readlink(path,buf,bufsize);
}



//int getdir (const char *, fuse_dirh_t, fuse_dirfil_t)
//{
//    return 0;
//}



int mknod (const char *pathname, mode_t mode, dev_t dev)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->mknod(pathname,mode,dev);
}



int mkdir (const char *pathname, mode_t mode)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->mkdir(pathname,mode);
}



int unlink (const char *pathname)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->unlink(pathname);
}



int rmdir (const char *pathname)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->rmdir(pathname);
}



int symlink (const char *oldpath, const char *newpath)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->symlink(oldpath,newpath);
}



int rename (const char *oldpath, const char *newpath)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->rename(oldpath,newpath);
}



int link (const char *oldpath, const char *newpath)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->link(oldpath,newpath);
}



int chmod (const char *path, mode_t mode)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->chmod(path,mode);
}



int chown (const char *path, uid_t owner, gid_t group)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->chown(path,owner,group);
}



int truncate (const char *path, off_t length)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->truncate(path,length);
}



//int utime (const char *, struct utimbuf *)
//{
//    return 0;
//}



int open (const char *pathname, struct fuse_file_info *info)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->open(pathname,info);
}



int read (const char *pathname,
                        char *buf,
                        size_t bufsize,
                        off_t offset,
                        struct fuse_file_info *info)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->read(pathname,buf,bufsize,offset,info);
}



int write (const char *pathname,
                        const char *buf,
                        size_t bufsize,
                        off_t offset,
                      struct fuse_file_info *info)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->write(pathname,buf,bufsize,offset,info);
}



int statfs (const char *path, struct statvfs *buf)
{
   fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->statfs(path,buf);
}



int flush (const char *path, struct fuse_file_info *info)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->flush(path,info);
}



int release (const char *path, struct fuse_file_info *info)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->release(path,info);
}



int fsync (const char *path,
                        int syncdata,
                        struct fuse_file_info *info)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->fsync(path,syncdata,info);
}



int setxattr (const char *pathname,
                            const char *key,
                            const char *value,
                            size_t bufsize,
                            int unknown)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->setxattr(pathname,key,value,bufsize,unknown);
}



int getxattr (const char *pathname,
                        const char *key,
                        char *buf,
                        size_t bufsize)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->getxattr(pathname,key,buf,bufsize);
}



int listxattr (const char *pathname,
                            char *buf,
                            size_t bufsize)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->listxattr(pathname,buf,bufsize);
}



int removexattr (const char *pathname, const char *key)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->removexattr(pathname,key);
}



int opendir (const char *path,
                            struct fuse_file_info *fi)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->opendir(path,fi);
}



int readdir (const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->readdir(path,buf,filler,offset,fi);
}



int releasedir (const char *path, struct fuse_file_info *fi)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->releasedir(path,fi);
}



int fsyncdir (const char *path, int datasync,
                            struct fuse_file_info *fi)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->fsyncdir(path,datasync,fi);
}



void *init (struct fuse_conn_info *conn)
{
    std::cout << "INIT!!\n\n";
    fuse_context*       ctx  = fuse_get_context();
    OpenbookFS_Init*    init = static_cast<OpenbookFS_Init*>(ctx->private_data);
    return init ? init->create() : 0;
}



void destroy (void* private_data)
{
    OpenbookFS* fs = static_cast<OpenbookFS*>(private_data);
    if( fs ) delete fs;
}



int access (const char *path, int mode)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->access(path,mode);
}



int create (const char *path,
                        mode_t mode,
                        struct fuse_file_info *fi)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->create(path,mode,fi);
}



int ftruncate (const char *path,
                            off_t length,
                            struct fuse_file_info *fi)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->ftruncate(path,length,fi);
}



int fgetattr (   const char *path,
                            struct stat *sf,
                            struct fuse_file_info *fi)
{
    fuse_context* ctx = fuse_get_context();
    OpenbookFS*    fs = static_cast<OpenbookFS*>(ctx->private_data);
    return fs->fgetattr(path,sf,fi);
}



int lock (const char *, struct fuse_file_info *, int cmd,
                     struct flock *)
{
    return 0;
}



int utimens (const char *, const struct timespec tv[2])
{
    return 0;
}



int bmap (const char *, size_t blocksize, uint64_t *idx)
{
    return 0;
}



int ioctl (const char *, int cmd, void *arg,
                      struct fuse_file_info *, unsigned int flags, void *data)
{
    return 0;
}



int poll ( const char *, struct fuse_file_info *,
                      struct fuse_pollhandle *ph, unsigned *reventsp)
{
    return 0;
}



} // namespace fuseops
} // namespace client
} // namespace filesystem
} // namespace openbook



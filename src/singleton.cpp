/*
 * singleton.cpp
 *
 *  Created on: Jun 7, 2012
 *      Author: josh
 */

#include "singleton.h"
#include "OpenbookFS.h"


namespace openbookfs
{
    OpenbookFS* g_singleton;
}


using namespace openbookfs;

void openbookfs_createSingleton( int argc, char** argv )
{
    g_singleton = new OpenbookFS(argc,argv);
}

void openbookfs_getFuseArgs( int* argc, char*** argv )
{
    g_singleton->getFuseArgs(argc,argv);
}

void openbookfs_destroySingleton()
{
    delete g_singleton;
}




int openbookfs_getattr (const char *path, struct stat *out)
{
    return g_singleton->getattr(path,out);
}



int openbookfs_readlink (const char *path, char *buf, size_t bufsize)
{
    return g_singleton->readlink(path,buf,bufsize);
}



//int openbookfs_getdir (const char *, fuse_dirh_t, fuse_dirfil_t)
//{
//    return 0;
//}



int openbookfs_mknod (const char *pathname, mode_t mode, dev_t dev)
{
    return g_singleton->mknod(pathname,mode,dev);
}



int openbookfs_mkdir (const char *pathname, mode_t mode)
{
    return g_singleton->mkdir(pathname,mode);
}



int openbookfs_unlink (const char *pathname)
{
    return g_singleton->unlink(pathname);
}



int openbookfs_rmdir (const char *pathname)
{
    return g_singleton->rmdir(pathname);
}



int openbookfs_symlink (const char *oldpath, const char *newpath)
{
    return g_singleton->symlink(oldpath,newpath);
}



int openbookfs_rename (const char *oldpath, const char *newpath)
{
    return g_singleton->rename(oldpath,newpath);
}



int openbookfs_link (const char *oldpath, const char *newpath)
{
    return g_singleton->link(oldpath,newpath);
}



int openbookfs_chmod (const char *path, mode_t mode)
{
    return g_singleton->chmod(path,mode);
}



int openbookfs_chown (const char *path, uid_t owner, gid_t group)
{
    return g_singleton->chown(path,owner,group);
}



int openbookfs_truncate (const char *path, off_t length)
{
    return g_singleton->truncate(path,length);
}



//int openbookfs_utime (const char *, struct utimbuf *)
//{
//    return 0;
//}



int openbookfs_open (const char *pathname, struct fuse_file_info *info)
{
    return g_singleton->open(pathname,info);
}



int openbookfs_read (const char *pathname,
                        char *buf,
                        size_t bufsize,
                        off_t offset,
                        struct fuse_file_info *info)
{
    return g_singleton->read(pathname,buf,bufsize,offset,info);
}



int openbookfs_write (const char *pathname,
                        const char *buf,
                        size_t bufsize,
                        off_t offset,
                      struct fuse_file_info *info)
{
    return g_singleton->write(pathname,buf,bufsize,offset,info);
}



int openbookfs_statfs (const char *path, struct statvfs *buf)
{
    return g_singleton->statfs(path,buf);
}



int openbookfs_flush (const char *path, struct fuse_file_info *info)
{
    return g_singleton->flush(path,info);
}



int openbookfs_release (const char *path, struct fuse_file_info *info)
{
    return g_singleton->release(path,info);
}



int openbookfs_fsync (const char *path,
                        int syncdata,
                        struct fuse_file_info *info)
{
    return g_singleton->fsync(path,syncdata,info);
}



int openbookfs_setxattr (const char *pathname,
                            const char *key,
                            const char *value,
                            size_t bufsize,
                            int unknown)
{
    return g_singleton->setxattr(pathname,key,value,bufsize,unknown);
}



int openbookfs_getxattr (const char *pathname,
                        const char *key,
                        char *buf,
                        size_t bufsize)
{
    return g_singleton->getxattr(pathname,key,buf,bufsize);
}



int openbookfs_listxattr (const char *pathname,
                            char *buf,
                            size_t bufsize)
{
    return g_singleton->listxattr(pathname,buf,bufsize);
}



int openbookfs_removexattr (const char *pathname, const char *key)
{
    return g_singleton->removexattr(pathname,key);
}



int openbookfs_opendir (const char *path,
                            struct fuse_file_info *fi)
{
    return g_singleton->opendir(path,fi);
}



int openbookfs_readdir (const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    return g_singleton->readdir(path,buf,filler,offset,fi);
}



int openbookfs_releasedir (const char *path, struct fuse_file_info *fi)
{
    return g_singleton->releasedir(path,fi);
}



int openbookfs_fsyncdir (const char *path, int datasync,
                            struct fuse_file_info *fi)
{
    return g_singleton->fsyncdir(path,datasync,fi);
}



void *openbookfs_init (struct fuse_conn_info *conn)
{
    return g_singleton;
}



void openbookfs_destroy (void* private_data)
{
    return;
}



int openbookfs_access (const char *path, int mode)
{
    return g_singleton->access(path,mode);
}



int openbookfs_create (const char *path,
                        mode_t mode,
                        struct fuse_file_info *fi)
{
    return g_singleton->create(path,mode,fi);
}



int openbookfs_ftruncate (const char *path,
                            off_t length,
                            struct fuse_file_info *fi)
{
    return g_singleton->ftruncate(path,length,fi);
}



int openbookfs_fgetattr (const char *, struct stat *, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_lock (const char *, struct fuse_file_info *, int cmd,
                     struct flock *)
{
    return 0;
}



int openbookfs_utimens (const char *, const struct timespec tv[2])
{
    return 0;
}



int openbookfs_bmap (const char *, size_t blocksize, uint64_t *idx)
{
    return 0;
}



int openbookfs_ioctl (const char *, int cmd, void *arg,
                      struct fuse_file_info *, unsigned int flags, void *data)
{
    return 0;
}



int openbookfs_poll ( const char *, struct fuse_file_info *,
                      struct fuse_pollhandle *ph, unsigned *reventsp)
{
    return 0;
}







/*
 * singleton.cpp
 *
 *  Created on: Jun 7, 2012
 *      Author: josh
 */

#include "singleton.h"
#include "OpenbookFS.h"





int openbookfs_getattr (const char *, struct stat *)
{
    return 0;
}



int openbookfs_readlink (const char *, char *, size_t)
{
    return 0;
}



int openbookfs_getdir (const char *, fuse_dirh_t, fuse_dirfil_t)
{
    return 0;
}



int openbookfs_mknod (const char *, mode_t, dev_t)
{
    return 0;
}



int openbookfs_mkdir (const char *, mode_t)
{
    return 0;
}



int openbookfs_unlink (const char *)
{
    return 0;
}



int openbookfs_rmdir (const char *)
{
    return 0;
}



int openbookfs_symlink (const char *, const char *)
{
    return 0;
}



int openbookfs_rename (const char *, const char *)
{
    return 0;
}



int openbookfs_link (const char *, const char *)
{
    return 0;
}



int openbookfs_chmod (const char *, mode_t)
{
    return 0;
}



int openbookfs_chown (const char *, uid_t, gid_t)
{
    return 0;
}



int openbookfs_truncate (const char *, off_t)
{
    return 0;
}



int openbookfs_utime (const char *, struct utimbuf *)
{
    return 0;
}



int openbookfs_open (const char *, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_read (const char *, char *, size_t, off_t,
                        struct fuse_file_info *)
{
    return 0;
}



int openbookfs_write (const char *, const char *, size_t, off_t,
                      struct fuse_file_info *)
{
    return 0;
}



int openbookfs_statfs (const char *, struct statvfs *)
{
    return 0;
}



int openbookfs_flush (const char *, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_release (const char *, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_fsync (const char *, int, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_setxattr (const char *, const char *, const char *, size_t, int)
{
    return 0;
}



int openbookfs_getxattr (const char *, const char *, char *, size_t)
{
    return 0;
}



int openbookfs_listxattr (const char *, char *, size_t)
{
    return 0;
}



int openbookfs_removexattr (const char *, const char *)
{
    return 0;
}



int openbookfs_opendir (const char *, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_readdir (const char *, void *, fuse_fill_dir_t, off_t,
                        struct fuse_file_info *)
{
    return 0;
}



int openbookfs_releasedir (const char *, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_fsyncdir (const char *, int, struct fuse_file_info *)
{
    return 0;
}



void *openbookfs_init (struct fuse_conn_info *conn)
{
    using namespace openbookfs;

    OpenbookFS* ctx = new OpenbookFS();
    return ctx;
}



void openbookfs_destroy (void* pData)
{
    using namespace openbookfs;

    OpenbookFS* ctx = static_cast<OpenbookFS*>(pData);
    delete ctx;
}



int openbookfs_access (const char *, int)
{
    return 0;
}



int openbookfs_create (const char *, mode_t, struct fuse_file_info *)
{
    return 0;
}



int openbookfs_ftruncate (const char *, off_t, struct fuse_file_info *)
{
    return 0;
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







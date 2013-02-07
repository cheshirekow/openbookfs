/*
 * singleton.h
 *
 *  Created on: Jun 7, 2012
 *      Author: josh
 */

#ifndef OPENBOOKFS_SINGLETON_H_
#define OPENBOOKFS_SINGLETON_H_

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 26
#endif

#include <fuse.h>

extern "C"
{
    void openbookfs_createSingleton( int argc, char** argv );
    void openbookfs_getFuseArgs( int* argc, char*** argv );
    void openbookfs_destroySingleton( );

    int openbookfs_getattr (const char *, struct stat *);
    int openbookfs_readlink (const char *, char *, size_t);
//  int openbookfs_getdir (const char *, fuse_dirh_t, fuse_dirfil_t);
    int openbookfs_mknod (const char *, mode_t, dev_t);
    int openbookfs_mkdir (const char *, mode_t);
    int openbookfs_unlink (const char *);
    int openbookfs_rmdir (const char *);
    int openbookfs_symlink (const char *, const char *);
    int openbookfs_rename (const char *, const char *);
    int openbookfs_link (const char *, const char *);
    int openbookfs_chmod (const char *, mode_t);
    int openbookfs_chown (const char *, uid_t, gid_t);
    int openbookfs_truncate (const char *, off_t);
//  int openbookfs_utime (const char *, struct utimbuf *);
    int openbookfs_open (const char *, struct fuse_file_info *);
    int openbookfs_read (const char *, char *, size_t, off_t,
                            struct fuse_file_info *);
    int openbookfs_write (const char *, const char *, size_t, off_t,
                          struct fuse_file_info *);
    int openbookfs_statfs (const char *, struct statvfs *);
    int openbookfs_flush (const char *, struct fuse_file_info *);
    int openbookfs_release (const char *, struct fuse_file_info *);
    int openbookfs_fsync (const char *, int, struct fuse_file_info *);
    int openbookfs_setxattr (const char *, const char *, const char *, size_t, int);
    int openbookfs_getxattr (const char *, const char *, char *, size_t);
    int openbookfs_listxattr (const char *, char *, size_t);
    int openbookfs_removexattr (const char *, const char *);
    int openbookfs_opendir (const char *, struct fuse_file_info *);
    int openbookfs_readdir (const char *, void *, fuse_fill_dir_t, off_t,
                            struct fuse_file_info *);
    int openbookfs_releasedir (const char *, struct fuse_file_info *);
    int openbookfs_fsyncdir (const char *, int, struct fuse_file_info *);
    void *openbookfs_init (struct fuse_conn_info *conn);
    void openbookfs_destroy (void *);
    int openbookfs_access (const char *, int);
    int openbookfs_create (const char *, mode_t, struct fuse_file_info *);
    int openbookfs_ftruncate (const char *, off_t, struct fuse_file_info *);
    int openbookfs_fgetattr (const char *, struct stat *, struct fuse_file_info *);
    int openbookfs_lock (const char *, struct fuse_file_info *, int cmd,
                         struct flock *);
    int openbookfs_utimens (const char *, const struct timespec tv[2]);
    int openbookfs_bmap (const char *, size_t blocksize, uint64_t *idx);
    int openbookfs_ioctl (const char *, int cmd, void *arg,
                          struct fuse_file_info *, unsigned int flags, void *data);
    int openbookfs_poll ( const char *, struct fuse_file_info *,
                          struct fuse_pollhandle *ph, unsigned *reventsp);
}


#endif /* SINGLETON_H_ */

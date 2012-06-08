/*
 * main.cpp
 *
 *  Created on: Jun 7, 2012
 *      Author: josh
 */

#include <cstdlib>
#include "singleton.h"


static fuse_operations openbookfs_oper;




int main(int argc, char** argv)
{

    openbookfs_oper.getattr     = openbookfs_getattr;
    openbookfs_oper.readlink    = openbookfs_readlink;
    openbookfs_oper.getdir      = openbookfs_getdir;
    openbookfs_oper.mknod       = openbookfs_mknod;
    openbookfs_oper.mkdir       = openbookfs_mkdir;
    openbookfs_oper.unlink      = openbookfs_unlink;
    openbookfs_oper.rmdir       = openbookfs_rmdir;
    openbookfs_oper.symlink     = openbookfs_symlink;
    openbookfs_oper.rename      = openbookfs_rename;
    openbookfs_oper.link        = openbookfs_link;
    openbookfs_oper.chmod       = openbookfs_chmod;
    openbookfs_oper.chown       = openbookfs_chown;
    openbookfs_oper.truncate    = openbookfs_truncate;
    openbookfs_oper.utime       = openbookfs_utime;
    openbookfs_oper.open        = openbookfs_open;
    openbookfs_oper.read        = openbookfs_read;
    openbookfs_oper.write       = openbookfs_write;
    openbookfs_oper.statfs      = openbookfs_statfs;
    openbookfs_oper.flush       = openbookfs_flush;
    openbookfs_oper.release     = openbookfs_release;
    openbookfs_oper.fsync       = openbookfs_fsync;
#ifdef HAVE_SETXATTR
    openbookfs_oper.setxattr    = openbookfs_setxattr;
    openbookfs_oper.getxattr    = openbookfs_getxattr;
    openbookfs_oper.listxattr   = openbookfs_listxattr;
    openbookfs_oper.removexattr = openbookfs_removexattr;
#endif
    openbookfs_oper.opendir     = openbookfs_opendir;
    openbookfs_oper.readdir     = openbookfs_readdir;
    openbookfs_oper.releasedir  = openbookfs_releasedir;
    openbookfs_oper.fsyncdir    = openbookfs_fsyncdir;
    openbookfs_oper.init        = openbookfs_init;
    openbookfs_oper.destroy     = openbookfs_destroy;
    openbookfs_oper.access      = openbookfs_access;
    openbookfs_oper.create      = openbookfs_create;
    openbookfs_oper.ftruncate   = openbookfs_ftruncate;
    openbookfs_oper.fgetattr    = openbookfs_fgetattr;
    openbookfs_oper.lock        = openbookfs_lock;
    openbookfs_oper.utimens     = openbookfs_utimens;
    openbookfs_oper.bmap        = openbookfs_bmap;
    openbookfs_oper.ioctl       = openbookfs_ioctl;
    openbookfs_oper.poll        = openbookfs_poll;

    umask(0);
    return fuse_main(argc, argv, &openbookfs_oper, NULL);
}

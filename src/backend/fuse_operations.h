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
 *  @file   src/client_fs/fuse_operations.h
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_FUSE_OPERATIONS_H_
#define OPENBOOK_FS_FUSE_OPERATIONS_H_

#include "fuse_include.h"


namespace   openbook {
namespace filesystem {


void setFuseOps( fuse_operations& fuse_ops );

} // namespace filesystem
} // namespace openbook

namespace   openbook {
namespace filesystem {

/// encapsulates global functions which simply extract the fuse context
/// and then call the corresponding fuction of OpenbookFS
namespace   fuse_ops {



int getattr (const char *, struct stat *);
int readlink (const char *, char *, size_t);
//  int getdir (const char *, fuse_dirh_t, fuse_dirfil_t);
int mknod (const char *, mode_t, dev_t);
int mkdir (const char *, mode_t);
int unlink (const char *);
int rmdir (const char *);
int symlink (const char *, const char *);
int rename (const char *, const char *);
int link (const char *, const char *);
int chmod (const char *, mode_t);
int chown (const char *, uid_t, gid_t);
int truncate (const char *, off_t);
//  int utime (const char *, struct utimbuf *);
int open (const char *, struct fuse_file_info *);
int read (const char *, char *, size_t, off_t,
                        struct fuse_file_info *);
int write (const char *, const char *, size_t, off_t,
                      struct fuse_file_info *);
int statfs (const char *, struct statvfs *);
int flush (const char *, struct fuse_file_info *);
int release (const char *, struct fuse_file_info *);
int fsync (const char *, int, struct fuse_file_info *);
int setxattr (const char *, const char *, const char *, size_t, int);
int getxattr (const char *, const char *, char *, size_t);
int listxattr (const char *, char *, size_t);
int removexattr (const char *, const char *);
int opendir (const char *, struct fuse_file_info *);
int readdir (const char *, void *, fuse_fill_dir_t, off_t,
                        struct fuse_file_info *);
int releasedir (const char *, struct fuse_file_info *);
int fsyncdir (const char *, int, struct fuse_file_info *);
void *init (struct fuse_conn_info *conn);
void destroy (void *);
int access (const char *, int);
int create (const char *, mode_t, struct fuse_file_info *);
int ftruncate (const char *, off_t, struct fuse_file_info *);
int fgetattr (const char *, struct stat *, struct fuse_file_info *);
int lock (const char *, struct fuse_file_info *, int cmd,
                     struct flock *);
int utimens (const char *, const struct timespec tv[2]);
int bmap (const char *, size_t blocksize, uint64_t *idx);
int ioctl (const char *, int cmd, void *arg,
                      struct fuse_file_info *, unsigned int flags, void *data);
int poll ( const char *, struct fuse_file_info *,
                      struct fuse_pollhandle *ph, unsigned *reventsp);



} // namespace fuseops
} // namespace filesystem
} // namespace openbook






#endif // FUSE_OPERATIONS_H_

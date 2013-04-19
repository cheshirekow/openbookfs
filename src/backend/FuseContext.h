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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/FuseContext.h
 *
 *  @date   Apr 16, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_FUSECONTEXT_H_
#define OPENBOOK_FS_FUSECONTEXT_H_

#include <string>
#include <sys/types.h>
#include <boost/filesystem.hpp>
#include "fuse_include.h"



namespace   openbook {
namespace filesystem {

class Backend;

/// Main fuse context for the openbook filesystem
/**
 *  This is the object that is stored in the private data structure of the
 *  fuse context, and manages the interaction between fuse operations and
 *  communication with the server
 */
class FuseContext
{
    public:
        typedef boost::filesystem::path Path_t;

    private:
        Backend*    m_backend;
        Path_t      m_dataDir;
        Path_t      m_realRoot;

        int  result_or_errno(int result);

    public:
        FuseContext(Backend*, const std::string& relpath );

        ~FuseContext();

        /// Create a file node
        /**
         *
         * @param path	the path to the filesystem node
         * @param mode  specifies both permission and type of file to create
         * @param dev   version number if a special file
         * @return 0 on success, -1 on error (and errno set)
         *
         * @see mknod(2): The system call mknod() creates a file system node
         * (file, device special file or named pipe) named pathname, with
         * attributes specified by mode and dev.
         *
         * @p mode specifies the file type by bitwise ORing one of the
         * following:
         *  bitflag   | meaning
         *  ----------|----------------------------------
         * 	S_IFREG   | regular file
         * 	S_IFCHR   | character special file
         * 	S_IFBLK   | block special file
         * 	S_IFIFO   | named pipe
         * 	S_IFSOCK  | unix domain socket
         *
         * @p dev is only used if @p mode is one of S_IFCHR or S_IFBLK, in which
         * case it specifies the major and minor numbers of the newly created
         * special block file
         *
         *
         * This is called for creation of all non-directory, non-symlink
         * nodes.  If the filesystem defines a create() method, then for
         * regular files that will be called instead.
         *
         * The openbookfs also creates the file obfs.sqlite, which is where
         * meta data is stored
         */
        int mknod (const char * path, mode_t mode, dev_t dev);

        /// Create and open a file
        /**
         * If the file does not exist, first create it with the specified
         * mode, and then open it.
         *
         * If this method is not implemented or under Linux kernel
         * versions earlier than 2.6.15, the mknod() and open() methods
         * will be called instead.
         *
         * Introduced in version 2.5
         */
        int create (const char *, mode_t, struct fuse_file_info *);

        /// File open operation
        /**
         * No creation (O_CREAT, O_EXCL) and by default also no
         * truncation (O_TRUNC) flags will be passed to open(). If an
         * application specifies O_TRUNC, fuse first calls truncate()
         * and then open(). Only if 'atomic_o_trunc' has been
         * specified and kernel version is 2.6.24 or later, O_TRUNC is
         * passed on to open.
         *
         * Unless the 'default_permissions' mount option is given,
         * open should check if the operation is permitted for the
         * given flags. Optionally open may also return an arbitrary
         * filehandle in the fuse_file_info structure, which will be
         * passed to all file operations.
         *
         * Changed in version 2.2
         *
         * OpenbookFS simply calls open and returns the actual file handle in
         * the @p fi->fh field. We do not mark the file as dirty, even if it's
         * opened for read write, unless an actual write is performed
         */
        int open (const char *, struct fuse_file_info *);

        /// Read data from an open file
        /**
         * Read should return exactly the number of bytes requested except
         * on EOF or error, otherwise the rest of the data will be
         * substituted with zeroes.  An exception to this is when the
         * 'direct_io' mount option is specified, in which case the return
         * value of the read system call will reflect the return value of
         * this operation.
         *
         * Changed in version 2.2
         */
        int read (const char *, char *, size_t, off_t,
                 struct fuse_file_info *);

        /// Write data to an open file
        /**
         * Write should return exactly the number of bytes requested
         * except on error.  An exception to this is when the 'direct_io'
         * mount option is specified (see read operation).
         *
         * Changed in version 2.2
         *
         * this is one method which marks an open fd as changed, initiating
         * a version increment
         */
        int write (const char *, const char *, size_t, off_t,
                  struct fuse_file_info *);

        /// Change the size of a file
        int truncate (const char *, off_t);

        /// Change the size of an open file
        /**
         * This method is called instead of the truncate() method if the
         * truncation was invoked from an ftruncate() system call.
         *
         * If this method is not implemented or under Linux kernel
         * versions earlier than 2.6.15, the truncate() method will be
         * called instead.
         *
         * Introduced in version 2.5
         */
        int ftruncate (const char *, off_t, struct fuse_file_info *);

        /// Release an open file
        /**
         * Release is called when there are no more references to an open
         * file: all file descriptors are closed and all memory mappings
         * are unmapped.
         *
         * For every open() call there will be exactly one release() call
         * with the same flags and file descriptor.  It is possible to
         * have a file opened more than once, in which case only the last
         * release will mean, that no more reads/writes will happen on the
         * file.  The return value of release is ignored.
         *
         * Changed in version 2.2
         */
        int release (const char *, struct fuse_file_info *);









        /** Get file attributes.
         *
         * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
         * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
         * mount option is given.
         */
        int getattr (const char *, struct stat *);

        /** Read the target of a symbolic link
         *
         * The buffer should be filled with a null terminated string.  The
         * buffer size argument includes the space for the terminating
         * null character.  If the linkname is too long to fit in the
         * buffer, it should be truncated.  The return value should be 0
         * for success.
         */
        int readlink (const char *, char *, size_t);

        /* Deprecated, use readdir() instead */
        // int getdir (const char *, fuse_dirh_t, fuse_dirfil_t);



        /** Create a directory
         *
         * Note that the mode argument may not have the type specification
         * bits set, i.e. S_ISDIR(mode) can be false.  To obtain the
         * correct directory type bits use  mode|S_IFDIR
         * */
        int mkdir (const char *, mode_t);

        /** Remove a file */
        int unlink (const char *);

        /** Remove a directory */
        int rmdir (const char *);

        /** Create a symbolic link */
        int symlink (const char *, const char *);

        /** Rename a file */
        int rename (const char *, const char *);

        /** Create a hard link to a file */
        int link (const char *, const char *);

        /** Change the permission bits of a file */
        int chmod (const char *, mode_t);

        /** Change the owner and group of a file */
        int chown (const char *, uid_t, gid_t);

        /** Change the access and/or modification times of a file
         *
         * Deprecated, use utimens() instead.
         */
        //int utime (const char *, struct utimbuf *);





        /** Get file system statistics
         *
         * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
         *
         * Replaced 'struct statfs' parameter with 'struct statvfs' in
         * version 2.5
         */
        int statfs (const char *, struct statvfs *);

        /** Possibly flush cached data
         *
         * BIG NOTE: This is not equivalent to fsync().  It's not a
         * request to sync dirty data.
         *
         * Flush is called on each close() of a file descriptor.  So if a
         * filesystem wants to return write errors in close() and the file
         * has cached dirty data, this is a good place to write back data
         * and return any errors.  Since many applications ignore close()
         * errors this is not always useful.
         *
         * NOTE: The flush() method may be called more than once for each
         * open().  This happens if more than one file descriptor refers
         * to an opened file due to dup(), dup2() or fork() calls.  It is
         * not possible to determine if a flush is final, so each flush
         * should be treated equally.  Multiple write-flush sequences are
         * relatively rare, so this shouldn't be a problem.
         *
         * Filesystems shouldn't assume that flush will always be called
         * after some writes, or that if will be called at all.
         *
         * Changed in version 2.2
         */
        int flush (const char *, struct fuse_file_info *);

        /** Synchronize file contents
         *
         * If the datasync parameter is non-zero, then only the user data
         * should be flushed, not the meta data.
         *
         * Changed in version 2.2
         */
        int fsync (const char *, int, struct fuse_file_info *);

        /** Set extended attributes */
        int setxattr (const char *, const char *, const char *, size_t, int);

        /** Get extended attributes */
        int getxattr (const char *, const char *, char *, size_t);

        /** List extended attributes */
        int listxattr (const char *, char *, size_t);

        /** Remove extended attributes */
        int removexattr (const char *, const char *);

        /** Open directory
         *
         * Unless the 'default_permissions' mount option is given,
         * this method should check if opendir is permitted for this
         * directory. Optionally opendir may also return an arbitrary
         * filehandle in the fuse_file_info structure, which will be
         * passed to readdir, closedir and fsyncdir.
         *
         * Introduced in version 2.3
         */
        int opendir (const char *, struct fuse_file_info *);

        /** Read directory
         *
         * This supersedes the old getdir() interface.  New applications
         * should use this.
         *
         * The filesystem may choose between two modes of operation:
         *
         * 1) The readdir implementation ignores the offset parameter, and
         * passes zero to the filler function's offset.  The filler
         * function will not return '1' (unless an error happens), so the
         * whole directory is read in a single readdir operation.  This
         * works just like the old getdir() method.
         *
         * 2) The readdir implementation keeps track of the offsets of the
         * directory entries.  It uses the offset parameter and always
         * passes non-zero offset to the filler function.  When the buffer
         * is full (or an error happens) the filler function will return
         * '1'.
         *
         * Introduced in version 2.3
         */
        int readdir (const char *, void *, fuse_fill_dir_t, off_t,
                struct fuse_file_info *);

        /** Release directory
         *
         * Introduced in version 2.3
         */
        int releasedir (const char *, struct fuse_file_info *);

        /** Synchronize directory contents
         *
         * If the datasync parameter is non-zero, then only the user data
         * should be flushed, not the meta data
         *
         * Introduced in version 2.3
         */
        int fsyncdir (const char *, int, struct fuse_file_info *);

        /**
         * Check file access permissions
         *
         * This will be called for the access() system call.  If the
         * 'default_permissions' mount option is given, this method is not
         * called.
         *
         * This method is not called under Linux kernel versions 2.4.x
         *
         * Introduced in version 2.5
         */
        int access (const char *, int);





        /**
         * Get attributes from an open file
         *
         * This method is called instead of the getattr() method if the
         * file information is available.
         *
         * Currently this is only called after the create() method if that
         * is implemented (see above).  Later it may be called for
         * invocations of fstat() too.
         *
         * Introduced in version 2.5
         */
        int fgetattr (const char *, struct stat *, struct fuse_file_info *);

        /**
         * Perform POSIX file locking operation
         *
         * The cmd argument will be either F_GETLK, F_SETLK or F_SETLKW.
         *
         * For the meaning of fields in 'struct flock' see the man page
         * for fcntl(2).  The l_whence field will always be set to
         * SEEK_SET.
         *
         * For checking lock ownership, the 'fuse_file_info->owner'
         * argument must be used.
         *
         * For F_GETLK operation, the library will first check currently
         * held locks, and if a conflicting lock is found it will return
         * information without calling this method.  This ensures, that
         * for local locks the l_pid field is correctly filled in.  The
         * results may not be accurate in case of race conditions and in
         * the presence of hard links, but it's unlikly that an
         * application would rely on accurate GETLK results in these
         * cases.  If a conflicting lock is not found, this method will be
         * called, and the filesystem may fill out l_pid by a meaningful
         * value, or it may leave this field zero.
         *
         * For F_SETLK and F_SETLKW the l_pid field will be set to the pid
         * of the process performing the locking operation.
         *
         * Note: if this method is not implemented, the kernel will still
         * allow file locking to work locally.  Hence it is only
         * interesting for network filesystems and similar.
         *
         * Introduced in version 2.6
         */
        int lock (const char *, struct fuse_file_info *, int cmd,
                 struct flock *);

        /**
         * Change the access and modification times of a file with
         * nanosecond resolution
         *
         * Introduced in version 2.6
         */
        int utimens (const char *, const struct timespec tv[2]);

        /**
         * Map block index within file to block index within device
         *
         * Note: This makes sense only for block device backed filesystems
         * mounted with the 'blkdev' option
         *
         * Introduced in version 2.6
         */
        int bmap (const char *, size_t blocksize, uint64_t *idx);

        /**
         * Ioctl
         *
         * flags will have FUSE_IOCTL_COMPAT set for 32bit ioctls in
         * 64bit environment.  The size and direction of data is
         * determined by _IOC_*() decoding of cmd.  For _IOC_NONE,
         * data will be NULL, for _IOC_WRITE data is out area, for
         * _IOC_READ in area and if both are set in/out area.  In all
         * non-NULL cases, the area is of _IOC_SIZE(cmd) bytes.
         *
         * Introduced in version 2.8
         */
        int ioctl (const char *, int cmd, void *arg,
                  struct fuse_file_info *, unsigned int flags, void *data);

        /**
         * Poll for IO readiness events
         *
         * Note: If ph is non-NULL, the client should notify
         * when IO readiness events occur by calling
         * fuse_notify_poll() with the specified ph.
         *
         * Regardless of the number of times poll with a non-NULL ph
         * is received, single notification is enough to clear all.
         * Notifying more times incurs overhead but doesn't harm
         * correctness.
         *
         * The callee is responsible for destroying ph with
         * fuse_pollhandle_destroy() when no longer in use.
         *
         * Introduced in version 2.8
         */
        int poll (const char *, struct fuse_file_info *,
                 struct fuse_pollhandle *ph, unsigned *reventsp);
};


/// simply stores initializer options for the OpenbookFS object
struct FuseContext_Init
{
    Backend*        backend;    //< backend object
    std::string     reldir;     //< relative directory of logical fs

    FuseContext* create()
    {
        return new FuseContext(backend,reldir);
    }
};

} // namespace filesystem
} // namespace openbook
















#endif // FUSECONTEXT_H_

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

namespace openbookfs
{


int OpenbookFS::result_or_errno(int result)
{
    if(result < 0)
        return -errno;
    else
        return result;
}

const char* OpenbookFS::wrap( const char* path )
{
    return (m_dataDir / path).c_str();
}


OpenbookFS::OpenbookFS(int argc, char** argv)
{
    namespace fs = boost::filesystem;

    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

    time_t      rawtime;
    tm*         timeinfo;
    char        currentYear[5];

    ::time( &rawtime );
    timeinfo = ::localtime( &rawtime );
    strftime (currentYear,5,"%Y",timeinfo);

    fs::path homeDir     = getenv("HOME");
    fs::path dfltDataDir = homeDir / "OpenbookFS::data";
    fs::path dfltMountDir= homeDir / "OpenbookFS::mount";


    std::stringstream sstream;
    sstream << "Openbook Filesystem\n"
            << "Copyright (c) 2012-" << currentYear
            << " Josh Bialkowski <josh.bialkowski@gmail.com>";

    // Define the command line object, and insert a message
    // that describes the program. The "Command description message"
    // is printed last in the help text. The second argument is the
    // delimiter (usually space) and the last one is the version number.
    // The CmdLine object parses the argv array based on the Arg objects
    // that it contains.
    TCLAP::CmdLine cmd(sstream.str().c_str(), ' ', "0.1.0");

    // Define a value argument and add it to the command line.
    // A value arg defines a flag and a type of value that it expects,
    // such as "-n Bishop".
    TCLAP::ValueArg<std::string> dataDirArg(
            "a",    // ............................................. short flag
            "data", // .............................................. long flag
            "data directory on the "    // .......... user-readable description
                "actual file system",
            false,   // .............................................. required?
            dfltDataDir.string(),   // .......................... default value
            "path"  // ..................................... user readable type
            );

    TCLAP::UnlabeledValueArg<std::string> mountArg(
            "mount_point",  // ........................................... name
            "where to mount the filesystem ",   // .. user-readable description
            false,   // .............................................. required?
            dfltMountDir.string(),   // ......................... default value
            "path"  // ..................................... user readable type
            );

    TCLAP::SwitchArg fuseHelpArg(
            "H",
            "fusehelp",
            "show help output and options from fuse",
            false
            );

    TCLAP::SwitchArg fuseVersionArg(
            "V",
            "fuseversion",
            "show version info from fuse",
            false
            );

    TCLAP::SwitchArg debugArg(
            "d",
            "debug",
            "enable debug output (implies -f)",
            false
            );

    TCLAP::SwitchArg foregroundArg(
            "f",
            "foreground",
            "foreground operation (don't daemonize)",
            false
            );

    TCLAP::SwitchArg singlethreadArg(
            "s",
            "singlethread",
            "disable multi-threaded operation",
            false
            );

    TCLAP::MultiArg<std::string> optionArgs(
            "o",
            "options",
            "additional options for fuse (--fusehelp for more info)",
            false,
            "strings"
            );


    // Add the argument nameArg to the CmdLine object. The CmdLine object
    // uses this Arg to parse the command line.
    cmd.add( mountArg );
    cmd.add( dataDirArg );
    cmd.add( fuseHelpArg );
    cmd.add( fuseVersionArg );
    cmd.add( debugArg );
    cmd.add( foregroundArg );
    cmd.add( singlethreadArg );
    cmd.add( optionArgs );

    // Parse the argv array.
    cmd.parse( argc, argv );

    // Get the value parsed by each arg.
    m_dataDir  = dataDirArg.getValue();

    // Generate a command line string for fuse
    std::list<std::string>  fuse_argv;

    fuse_argv.push_back(cmd.getProgramName());

    if( !( fuseHelpArg.getValue() || fuseVersionArg.getValue() ) )
        fuse_argv.push_back(mountArg.getValue());

    if(fuseHelpArg.getValue())
        fuse_argv.push_back("-h");

    if(fuseVersionArg.getValue())
        fuse_argv.push_back("-V");

    if(debugArg.getValue())
        fuse_argv.push_back("-d");

    if(foregroundArg.getValue())
        fuse_argv.push_back("-f");

    if(singlethreadArg.getValue())
        fuse_argv.push_back("-s");

    for(int i=0; i < optionArgs.getValue().size(); i++)
    {
        fuse_argv.push_back("-o");
        fuse_argv.push_back( optionArgs.getValue()[i] );
    }

    // calculate the number of bytes we need to store argv
    std::list<std::string>::iterator    itArgv;
    int                                 nChars = 0;
    for(itArgv = fuse_argv.begin(); itArgv != fuse_argv.end(); itArgv++)
        nChars += itArgv->length() + 1;

    // allocate such a character array
    char* argv_buf  = new char[nChars];
    int   iArg      = 0;
    int   iBuf      = 0;
    m_fuse_argc     = fuse_argv.size();
    m_fuse_argv     = new char*[m_fuse_argc];

    for(itArgv = fuse_argv.begin(); itArgv != fuse_argv.end(); itArgv++)
    {
        char*   ptrArg      = argv_buf + iBuf;
        int     argLen      = itArgv->length();
        m_fuse_argv[iArg++] = ptrArg;

        itArgv->copy(ptrArg,argLen);
        iBuf += argLen;

        argv_buf[iBuf] = '\0';
        iBuf ++;
    }

    std::cerr << "Finished building argument vector: \n   ";
    for(int i=0; i < nChars; i++)
    {
        if(argv_buf[i] != '\0')
            std::cerr << argv_buf[i];
        else
            std::cerr << " ";
    }
    std::cerr << std::endl;


    }

    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr   << "error: " << e.error() << " for arg "
                    << e.argId() << std::endl;
    }








}









OpenbookFS::~OpenbookFS()
{
    delete [] m_fuse_argv[0];
    delete [] m_fuse_argv;
}






void OpenbookFS::getFuseArgs(int* argc, char*** argv)
{
    std::cerr << "Returning fuse arguments (" << m_fuse_argc << "):\n";
    for(int i=0; i < m_fuse_argc; i++)
    {
        std::cerr << "   " << m_fuse_argv[i] << "\n";
    }
    std::cerr << std::endl;

    *argc = m_fuse_argc;
    *argv = m_fuse_argv;
}












int OpenbookFS::getattr (const char *path, struct stat *out)
{
    std::cerr << "getattr: " << path
                << "(" << (m_dataDir / path) << ")" << std::endl;

    return ::lstat( wrap(path), out ) ? -errno : 0;
}



int OpenbookFS::readlink (const char *path, char *buf, size_t bufsize)
{
    std::cerr << "readlink: " << path
                << "(" << (m_dataDir / path) << ")" << std::endl;

    return ::readlink(  wrap(path), buf, bufsize ) ? -errno : 0;
}





int OpenbookFS::mknod (const char *pathname, mode_t mode, dev_t dev)
{
    std::cerr << "mknod: " << pathname
                << "(" << (m_dataDir / pathname) << ")" << std::endl;

    return ::mknod( wrap(pathname), mode, dev ) ? -errno : 0;
}



int OpenbookFS::mkdir (const char *pathname, mode_t mode)
{
    std::cerr << "mkdir: " << pathname
                << "(" << (m_dataDir / pathname) << ")" << std::endl;

    return ::mkdir( wrap(pathname), mode|S_IFDIR ) ? -errno : 0;
}



int OpenbookFS::unlink (const char *pathname)
{
    std::cerr << "unlink: " << pathname
                << "(" << (m_dataDir / pathname) << ")" << std::endl;

    return result_or_errno(
                ::unlink( wrap(pathname) )
                 );
}



int OpenbookFS::rmdir (const char *pathname)
{
    std::cerr << "rmdir: " << pathname
                << "(" << (m_dataDir / pathname) << ")" << std::endl;

    return result_or_errno(
                ::rmdir( wrap(pathname) )
                 );
}



int OpenbookFS::symlink (const char *oldpath, const char *newpath)
{
    std::cerr << "simlink: "
                  "\n   old: " << oldpath               <<
                  "\n      : " << (m_dataDir/oldpath)   <<
                  "\n   new: " << newpath               <<
                  "\n      : " << (m_dataDir/newpath)   <<
                  std::endl;

    return result_or_errno(
            ::symlink(  wrap(oldpath),
                        wrap(newpath) )
                );
}



int OpenbookFS::rename (const char *oldpath, const char *newpath)
{
    std::cerr << "rename: "
                  "\n   old: " << oldpath               <<
                  "\n      : " << (m_dataDir/oldpath)   <<
                  "\n   new: " << newpath               <<
                  "\n      : " << (m_dataDir/newpath)   <<
                  std::endl;


    return ::rename(    wrap(oldpath),
                        wrap(newpath) );
}



int OpenbookFS::link (const char *oldpath, const char *newpath)
{
    std::cerr << "link: "
                  "\n   old: " << oldpath               <<
                  "\n      : " << (m_dataDir/oldpath)   <<
                  "\n   new: " << newpath               <<
                  "\n      : " << (m_dataDir/newpath)   <<
                  std::endl;


    return result_or_errno(
            ::link( wrap(oldpath),
                    wrap(newpath) )
                );
}



int OpenbookFS::chmod (const char *path, mode_t mode)
{
    std::cerr << "chmod: "
              "\n    path: " << path                <<
              "\n        : " << (m_dataDir/path)    <<
              "\n    mode: " << mode                <<
              std::endl;

    return result_or_errno(
            ::chmod(    wrap(path), mode )
             );
}



int OpenbookFS::chown (const char *path, uid_t owner, gid_t group)
{
    std::cerr << "chown: "
              "\n    path: " << path                <<
              "\n        : " << (m_dataDir/path)    <<
              "\n     uid: " << owner               <<
              "\n     gid: " << group               <<
              std::endl;

    return  result_or_errno(
            ::chown(    wrap(path),
                        owner,
                        group )
            );
}



int OpenbookFS::truncate (const char *path, off_t length)
{
    std::cerr << "truncate: "
              "\n      path: " << path                <<
              "\n          : " << (m_dataDir/path)    <<
              "\n    length: " << length              <<
              std::endl;

    return  result_or_errno(
        ::truncate( wrap(path), length  )
            );
}



//int OpenbookFS::utime (const char *, struct utimbuf *)
//{
//
//}



int OpenbookFS::open (const char *path, struct fuse_file_info *fi)
{
    std::cerr << "open: "
              "\n    path: " << path                <<
              "\n        : " << (m_dataDir/path)    <<
              std::endl;



    int fh = ::open(    wrap(path),
                        fi->flags );
    fi->fh = fh;

    return result_or_errno(( fh > 0 ) ? 0 : fh);
}



int OpenbookFS::read (const char *pathname,
                        char *buf,
                        size_t bufsize,
                        off_t offset,
                        struct fuse_file_info *fi)
{
    std::cerr << "read: "
              "\n    path: " << pathname                <<
              "\n        : " << (m_dataDir/pathname)    <<
              "\n    size: " << bufsize                 <<
              "\n     off: " << offset                  <<
              "\n      fh: " << fi->fh                  <<
              std::endl;

    if(fi->fh)
    {
        return result_or_errno(
                ::pread(fi->fh,buf,bufsize,offset) );
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
    {
        return result_or_errno( ::pwrite(fi->fh,buf,bufsize,offset) );
    }
    else
        return -EBADF;
}



int OpenbookFS::statfs (const char *path, struct statvfs *buf)
{
    std::cerr << "statfs: "
                "\n   path: " << path <<
                "\n       : " << (m_dataDir / path)
                << std::endl;

    return result_or_errno(
            ::statvfs(   wrap(path), buf )
             );
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
        int result = result_or_errno(
                ::close(fi->fh)
                );
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
    std::cerr << "opendir: "
                 "\n   path: " << path <<
                 "\n       : " << (m_dataDir / path) <<
                 std::endl;

    DIR* dir = ::opendir( wrap(path) );

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
    std::cerr << "readdir: "
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

    return result_or_errno(
            ::access(    wrap(pathname), mode )
             );
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

    fi->fh = ::creat( wrap(pathname), mode );
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
    std::cerr << "getattr: " << path
                << "(" << (m_dataDir / path) << ")" << std::endl;

    return result_or_errno(
            ::fstat( fi->fh, out )
             );
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



















} /* namespace openbookfs */

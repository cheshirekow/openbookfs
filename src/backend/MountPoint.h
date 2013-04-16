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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/MountPoint.h
 *
 *  @date   Apr 16, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_MOUNTPOINT_H_
#define OPENBOOK_FS_MOUNTPOINT_H_

#include <string>
#include <cpp-pthreads.h>
#include "fuse_include.h"

namespace   openbook {
namespace filesystem {

class Backend;

/// encapsulates the path to a mount point, the fuse channel, and fuse object
/// for the fuse filesystem mounted at that point
class MountPoint
{
    public:
        typedef std::vector<std::string> argv_t;

    private:
        pthreads::Thread  m_thread;   ///< the thread we run in
        std::string       m_mount;    ///< path to the mount point
        fuse_chan*        m_fuseChan; ///< channel from fuse_mount
        fuse*             m_fuse;     ///< fuse struct from fuse_new
        fuse_operations   m_ops;      ///< fuse operations
        bool              m_mt;       ///< use multi threaded loop
        std::string       m_reldir;   ///< where to serve files from
        argv_t            m_args;     ///< arguments passed to fuse

    public:
        /// initialize the structure
        MountPoint( const std::string& mount );

        /// starts fuse in it's own thread
        void mount(Backend* backend, const std::string& reldir,
                    int argc, char** argv);

        /// calls fusermount -u
        /**
         *  note: it seems that it would be reasonable to call fuse_exit
         *  from the main thread (i.e. the caller, here) but there might
         *  be a kernel bug that would cause a memory leak. Instead, we are
         *  safe and make a system call to run "fusermount -u" which should
         *  do everything correctly.
         *
         *  see: http://sourceforge.net/mailarchive/message.php?msg_id=28221264
         */
        void unmount();

        const std::string&      mountPoint() const { return m_mount;  }
        const std::string&      relDir()     const { return m_reldir; }
        argv_t::const_iterator  argv()       const { return m_args.begin(); }
        argv_t::const_iterator  argv_end()   const { return m_args.end();   }

    private:
        static void* dispatch_main(void* vp_mountpoint);
        void main();



};


}  //< namespace filesystem
}  //< namespace openbook















#endif // MOUNTPOINT_H_

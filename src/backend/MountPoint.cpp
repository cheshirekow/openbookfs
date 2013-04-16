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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/MountPoint.cpp
 *
 *  @date   Apr 16, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */


#include "fuse_operations.h"
#include "Backend.h"
#include "ExceptionStream.h"
#include "FuseContext.h"
#include "MountPoint.h"


namespace   openbook {
namespace filesystem {

MountPoint::MountPoint( Backend* backend, const std::string path ):
    m_path(path),
    m_fuseChan(0),
    m_fuse(0),
    m_backend(backend),
    m_mt(false)
{}

void MountPoint::mount(int argc, char** argv)
{
    // fuse arguments
    fuse_args args = {argc,argv,0};

    // create the mount point
    m_fuseChan = fuse_mount( m_path.c_str(), &args );
    if(!m_fuseChan)
        ex()() << "Failed to fuse_mount " << m_path;

    // initialize fuse_ops
    setFuseOps( m_ops );

    // create initializer object which is passed to fuse_ops::init
    FuseContext_Init init;
    init.backend = m_backend;

    // initialize fuse
    m_fuse = fuse_new(m_fuseChan,&args,&m_ops,sizeof(m_ops),&init);
    if( !m_fuse )
    {
        fuse_unmount( m_path.c_str(), m_fuseChan );
        m_fuseChan = 0;
        ex()() << "Failed to fuse_new";
    }

    // start the fuse event handler thread
    m_thread.launch(dispatch_main,this);
}

void MountPoint::unmount()
{
    std::string cmd = "fusermount -u " + m_path;
    std::cout << "Mointpoint::unmount: " << cmd;

    int result = system(cmd.c_str());
    if( result < 0 )
        ex()() << "Failed to unmount " << m_path;

    m_thread.join();
}

void* MountPoint::dispatch_main(void* vp_mountpoint)
{
    static_cast<MountPoint*>(vp_mountpoint)->main();
    return vp_mountpoint;
}

void MountPoint::main()
{
    std::cout << "MountPoint::main: " << (void*)this << "entering fuse loop\n";

    if( m_mt )
        fuse_loop_mt(m_fuse);
    else
        fuse_loop(m_fuse);

    std::cout << "MountPoint::main: " << (void*)this << "exiting fuse loop\n";
    fuse_unmount(m_path.c_str(),m_fuseChan);
    fuse_destroy(m_fuse);
}


}  //< namespace filesystem
}  //< namespace openbook








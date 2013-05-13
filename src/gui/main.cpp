
#include "mainwindow.h"
#include <QApplication>
#include <csignal>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>


#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"

#include "connection.h"
#include "Options.h"
#include "commands/Connect.h"
#include "commands/SaveConfig.h"
#include "commands/SetClientSocket.h"
#include "commands/SetDataDir.h"
#include "commands/SetDisplayName.h"
#include "commands/SetLocalSocket.h"
#include "commands/ListKnownPeers.h"
#include "commands/ListMounts.h"
#include "commands/LoadConfig.h"
#include "commands/SetRemoteSocket.h"
#include "commands/StartSync.h"

namespace   openbook {
namespace filesystem {

NotifyPipe* g_termNote;

} // namespace openbook
} // namespace filesystem

using namespace openbook;
using namespace filesystem;
using namespace gui;


void signal_callback( int signum )
{
    switch(signum)
    {
        case SIGINT:
        {
            std::cout << "Main thread received SIGINT, going to terminate"
                      << std::endl;
            g_termNote->notify();
            break;
        }

        default:
            std::cout << "Main thread: unexpected signal "
                      << signum <<", ignoring" << std::endl;
            break;
    }
}

int main(int argc, char *argv[])
{
    NotifyPipe termNote;
    g_termNote = &termNote;



    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}


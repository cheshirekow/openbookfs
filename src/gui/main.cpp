#include <csignal>
#include <iostream>
#include "global.h"
#include "mainwindow.h"
#include <QApplication>


namespace   openbook {
namespace filesystem {

NotifyPipe* g_termNote;      ///< pipe used to break out of select statements


} // namespace filesystem
} // namespace openbook


using namespace openbook;
using namespace filesystem;

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
    // for cancellation, opens an unnamed pipe that can be select()'ed to
    // preempt any socket calls
    NotifyPipe termNote;
    g_termNote = &termNote;

    // install signal handlers, when SIGINT is signalled (i.e. ctrl+c in the
    // terminal) the termNote is signalled any any blocking network calls
    // will break out, allowing the program to terminate
    signal(SIGINT,signal_callback);
    signal(SIGPIPE,signal_callback);


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}

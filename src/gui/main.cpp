
#include "mainwindow.h"
#include <QApplication>



namespace   openbook {
namespace filesystem {
namespace       gui {

int main(int argc, char *argv[])
{



    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}

}
}
}

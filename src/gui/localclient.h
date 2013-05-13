#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include <QWidget>
#include <QFileDialog>
#include "connection.h"
#include "commands/ListMounts.h"
#include "commands/ListKnownPeers.h"
#include "commands/SetDisplayName.h"
#include "commands/Connect.h"
#include "commands/StartSync.h"
#include "commands/SetDataDir.h"

#include "commands/Checkout.h"
#include "commands/Release.h"

namespace Ui {
class LocalClient;
}
namespace   openbook {
namespace filesystem {
namespace       gui {

class LocalClient : public QWidget
{
    Q_OBJECT
    
public:
    explicit LocalClient(QWidget *parent = 0);
    ~LocalClient();

    
private:
    Ui::LocalClient *ui;

    void initial_params();

    QStringList *mountList;

    QStringList *checked_out_files;

private slots:
    void update_params();
    void start_sync();
    void get_data_dir();
    void checkout_file();
    void release_file();
};
}
}
}

#endif // LOCALCLIENT_H

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

private slots:
    void update_params();
    void start_sync();
    void get_data_dir();
};
}
}
}

#endif // LOCALCLIENT_H

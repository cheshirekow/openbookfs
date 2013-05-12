#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include <QWidget>
#include "connection.h"
#include "commands/ListMounts.h"
#include "commands/ListKnownPeers.h"
#include "commands/SetDisplayName.h"
#include "commands/Connect.h"

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

private slots:
    void update_params();
};
}
}
}

#endif // LOCALCLIENT_H

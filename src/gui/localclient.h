#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include <QWidget>

#include "connection.h"

namespace Ui {
class LocalClient;
}

class LocalClient : public QWidget
{
    Q_OBJECT
    
public:
    explicit LocalClient(QWidget *parent = 0);
    ~LocalClient();
    
private:
    Ui::LocalClient *ui;

    Connection *connection;

private slots:
    void update_params();
};

#endif // LOCALCLIENT_H

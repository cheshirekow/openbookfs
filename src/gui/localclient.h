#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include <QWidget>


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
};

#endif // LOCALCLIENT_H

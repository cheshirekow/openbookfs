#include "localclient.h"
#include "ui_localclient.h"

namespace   openbook {
namespace filesystem {
namespace       gui {

LocalClient::LocalClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalClient)
{
    ui->setupUi(this);

    ui->display_i->SetText("Display Name: This is the name which will be displayed on the server and other clients to identify this client. If you only mount one file system, consider using the machine hostname for this field");

    Connect c;
    c.go();

    connect(ui->submit, SIGNAL(clicked()),this,SLOT(update_params()));

}

LocalClient::~LocalClient()
{
    delete ui;
}

void LocalClient::update_params()
{
    QString name = ui->display_name->text();



    ListKnownPeers l;
    l.go();

}

}
}
}

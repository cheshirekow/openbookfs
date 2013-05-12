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

    ui->display_i->SetText("Display Name: This is the name which will be displayed on the server and other clients to identify this client. If you only mount one file system, consider using the machine hostname.");

    //Connect c;
    //c.go();

    connect(ui->submit, SIGNAL(clicked()),this,SLOT(update_params()));

}

LocalClient::~LocalClient()
{
    delete ui;
}

void LocalClient::initial_params()
{

}

void LocalClient::update_params()
{
    Connect c;
    c.go();

    QString name = ui->display_name->text();

    ListMounts lm;
    lm.go();

    /*

    ListKnownPeers l;
    QStringList peers = l.go();

    ui->current_peers->clear();
    for(int i = 0; i < peers.length(); i++)
        ui->current_peers->addItem(peers.at(i));

   // ListMounts lm;
   // lm.go();

    SetDisplayName dn;
    dn.go("CLIENT_NAME_b");


    StartSync ss;
    ss.go();

    */

}

}
}
}

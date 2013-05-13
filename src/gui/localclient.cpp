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

    connect(ui->submit, SIGNAL(clicked()),this,SLOT(update_params()));
    connect(ui->sync_files,SIGNAL(clicked()),this,SLOT(start_sync()));
    connect(ui->data_dir,SIGNAL(clicked()),this,SLOT(get_data_dir()));
    connect(ui->checkout_file,SIGNAL(clicked()),this,SLOT(checkout_file()));
    connect(ui->release_file,SIGNAL(clicked()),this,SLOT(release_file()));

    mountList = new QStringList();
    checked_out_files = new QStringList();

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

    QString port = ui->local_port->text();

    Connect c(port);
    c.go(ui->remote_port->text(),"localhost");

    QString name = ui->display_name->text();

    ListMounts lm(port);
    QStringList mounts = lm.go();
    ui->mount_points->clear();
    mountList->clear();
    for(int i = 0; i < mounts.length();i++)
    {
        QListWidgetItem *item = new QListWidgetItem(mounts.at(i));
        ui->mount_points->addItem(item);
        ui->mount_points->setItemSelected(item,true);
        mountList->append(mounts.at(i));
    }


    ListKnownPeers l(port);
    QStringList peers = l.go();

    ui->current_peers->clear();
    for(int i = 0; i < peers.length(); i++)
        ui->current_peers->addItem(peers.at(i));


    SetDisplayName dn(port);
    dn.go(ui->display_name->text());


    SetDataDir data(port);
    QString t_data_dir = ui->data_directory->text();

    if(t_data_dir == "")
    {
        qDebug()<<"No Directory Chosen";
    }
    else{
        data.go(ui->data_directory->text());
    }



}

void LocalClient::start_sync()
{

    int current_peer = ui->current_peers->currentRow();
    if(current_peer == -1)
    {
       qDebug()<<"Peer not selected";
    }
    else{
        int peer = ui->current_peers->currentItem()->text().split(":").at(0).toInt();
        qDebug()<<"Sync to Peer:"<<peer;
        StartSync ss(ui->local_port->text());
        ss.go(peer);
    }
}
void LocalClient::get_data_dir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->data_directory->setText(dir);
}

void LocalClient::checkout_file()
{
    QString def = "/home";

    if(mountList->length()>0)
        def = mountList->first();
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Checkout File"), def, tr("Any File (*)"));

    qDebug()<<"Checking Out: "<<fileName;
    Checkout c(ui->local_port->text());
    c.go(fileName);

    if(checked_out_files->indexOf(fileName) == -1)
    {
        qDebug()<<"Index:"<<checked_out_files->indexOf(fileName);
        ui->checked_out_files->addItem(fileName);
        checked_out_files->append(fileName);
    }
}

void LocalClient::release_file()
{
    QListWidgetItem *w = ui->checked_out_files->currentItem();
    QString filename = w->text();

    checked_out_files->removeAt(checked_out_files->indexOf(filename));
    ui->checked_out_files->removeItemWidget(w);
    delete(w);
    qDebug()<<"Releasing: "<<filename;

    Release r(ui->local_port->text());
    r.go(filename);
}

}
}
}

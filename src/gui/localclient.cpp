#include "localclient.h"
#include "ui_localclient.h"

LocalClient::LocalClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalClient)
{
    ui->setupUi(this);

    ui->display_i->SetText("Display Name: This is the name which will be displayed on the server and other clients to identify this client. If you only mount one file system, consider using the machine hostname for this field");

}

LocalClient::~LocalClient()
{
    delete ui;
}

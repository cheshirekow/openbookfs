

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <sstream>
#include <string>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    connect(ui->connect,SIGNAL(clicked()),this,SLOT(connect_to()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connect_to()
{

    ui->connect->setVisible(false);

    localclient = new LocalClient();

    ui->layout->addWidget(localclient);
}




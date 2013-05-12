#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>





#include "localclient.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    
private:
    Ui::MainWindow *ui;

    QTcpSocket *socket;


    LocalClient *localclient;

private slots:
    void connect_to();



};

#endif // MAINWINDOW_H

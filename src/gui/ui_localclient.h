/********************************************************************************
** Form generated from reading UI file 'localclient.ui'
**
** Created: Mon Apr 29 15:50:40 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOCALCLIENT_H
#define UI_LOCALCLIENT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>
#include "info.h"

QT_BEGIN_NAMESPACE

class Ui_LocalClient
{
public:
    QLabel *label;
    QLineEdit *display_name;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *local_port;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *remote_port;
    QLabel *label_6;
    QLabel *label_7;
    QLineEdit *max_connections;
    QLabel *label_8;
    QLineEdit *data_directory;
    QPushButton *pushButton;
    Info *display_i;
    Info *data_dir_i;
    Info *local_port_i;
    Info *remote_port_i;
    Info *max_connections_i;

    void setupUi(QWidget *LocalClient)
    {
        if (LocalClient->objectName().isEmpty())
            LocalClient->setObjectName(QString::fromUtf8("LocalClient"));
        LocalClient->resize(450, 300);
        label = new QLabel(LocalClient);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 121, 21));
        display_name = new QLineEdit(LocalClient);
        display_name->setObjectName(QString::fromUtf8("display_name"));
        display_name->setGeometry(QRect(110, 10, 113, 22));
        label_2 = new QLabel(LocalClient);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 40, 101, 21));
        label_3 = new QLabel(LocalClient);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 70, 91, 16));
        local_port = new QLineEdit(LocalClient);
        local_port->setObjectName(QString::fromUtf8("local_port"));
        local_port->setGeometry(QRect(80, 90, 113, 22));
        label_4 = new QLabel(LocalClient);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(40, 90, 31, 21));
        label_5 = new QLabel(LocalClient);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 120, 91, 16));
        remote_port = new QLineEdit(LocalClient);
        remote_port->setObjectName(QString::fromUtf8("remote_port"));
        remote_port->setGeometry(QRect(80, 140, 113, 22));
        label_6 = new QLabel(LocalClient);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(40, 140, 31, 21));
        label_7 = new QLabel(LocalClient);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 180, 121, 16));
        max_connections = new QLineEdit(LocalClient);
        max_connections->setObjectName(QString::fromUtf8("max_connections"));
        max_connections->setGeometry(QRect(130, 180, 41, 22));
        label_8 = new QLabel(LocalClient);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(10, 210, 111, 16));
        data_directory = new QLineEdit(LocalClient);
        data_directory->setObjectName(QString::fromUtf8("data_directory"));
        data_directory->setGeometry(QRect(110, 40, 201, 22));
        pushButton = new QPushButton(LocalClient);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(310, 41, 71, 31));
        display_i = new Info(LocalClient);
        display_i->setObjectName(QString::fromUtf8("display_i"));
        display_i->setGeometry(QRect(230, 10, 21, 21));
        data_dir_i = new Info(LocalClient);
        data_dir_i->setObjectName(QString::fromUtf8("data_dir_i"));
        data_dir_i->setGeometry(QRect(380, 40, 21, 21));
        local_port_i = new Info(LocalClient);
        local_port_i->setObjectName(QString::fromUtf8("local_port_i"));
        local_port_i->setGeometry(QRect(200, 90, 21, 21));
        remote_port_i = new Info(LocalClient);
        remote_port_i->setObjectName(QString::fromUtf8("remote_port_i"));
        remote_port_i->setGeometry(QRect(200, 140, 21, 21));
        max_connections_i = new Info(LocalClient);
        max_connections_i->setObjectName(QString::fromUtf8("max_connections_i"));
        max_connections_i->setGeometry(QRect(180, 180, 21, 21));

        retranslateUi(LocalClient);

        QMetaObject::connectSlotsByName(LocalClient);
    } // setupUi

    void retranslateUi(QWidget *LocalClient)
    {
        LocalClient->setWindowTitle(QApplication::translate("LocalClient", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("LocalClient", "Display Name:", 0, QApplication::UnicodeUTF8));
        display_name->setText(QApplication::translate("LocalClient", "Test", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("LocalClient", "Data Directory:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("LocalClient", "Local:", 0, QApplication::UnicodeUTF8));
        local_port->setText(QApplication::translate("LocalClient", "3030", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("LocalClient", "Port:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("LocalClient", "Remote:", 0, QApplication::UnicodeUTF8));
        remote_port->setText(QApplication::translate("LocalClient", "3031", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("LocalClient", "Port:", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("LocalClient", "Max Connections:", 0, QApplication::UnicodeUTF8));
        max_connections->setText(QApplication::translate("LocalClient", "20", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("LocalClient", "Mount Points:", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("LocalClient", "Browse", 0, QApplication::UnicodeUTF8));
        display_i->setText(QString());
        data_dir_i->setText(QString());
        local_port_i->setText(QString());
        remote_port_i->setText(QString());
        max_connections_i->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class LocalClient: public Ui_LocalClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOCALCLIENT_H

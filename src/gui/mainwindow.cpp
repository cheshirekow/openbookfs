#include <iostream>

#include "global.h"
#include "FileDescriptor.h"
#include "ReferenceCounted.h"
#include "ExceptionStream.h"
#include "Marshall.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

// since this is a cpp file we can do this
using namespace openbook;
using namespace filesystem;

/// throws an exception if the message isn't of the right type
static void validate_message( RefPtr<AutoMessage> msg, MessageId expected )
{
    if( msg->type != expected )
    {
        ex()() << "Protocol Error: expected "
               << messageIdToString(expected)
               << " from peer, instead got"
               << messageIdToString(msg->type)
               << "(" << (int)msg->type << ")";
    }
}

/// perform handshake to notify client that we are a ui
/// todo: gabe, you may want to put this somewhere else unless all
/// callbacks will be in this cpp
static void handshake( Marshall& marshall )
{
    namespace msgs = messages;

    // trade public keys
    msgs::AuthRequest* authReq = new msgs::AuthRequest();
    authReq->set_display_name("GUI");
    authReq->set_public_key("UserInterface");
    marshall.writeMsg( authReq );

    RefPtr<AutoMessage> recv = marshall.read( );
    validate_message( recv, MSG_AUTH_REQ );
    authReq = static_cast<msgs::AuthRequest*>( recv->msg );

    std::cout << "Handshake with client:"
              << "\n" << authReq->display_name()
              << "\n" << authReq->public_key()
              << "\n";
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setDisplayName( const QString& name )
{
    // connect to the client and get a file descriptor using qt
    int sockfd = 0; //< not actually zero
    Marshall marshall;
    marshall.setFd(sockfd);
    handshake(marshall);

    // send the message
    messages::SetDisplayName* msg = new messages::SetDisplayName();
    msg->set_displayname( name.toAscii() );
    marshall.writeMsg(msg);

    // wait for the reply
    RefPtr<AutoMessage> reply = marshall.read();

    // print the result
    if( reply->type != MSG_UI_REPLY )
    {
        std::cerr << "Unexpected reply of type: "
                  << messageIdToString( reply->type )
                  << "\n";
    }
    else
    {
        messages::UserInterfaceReply* msg =
                static_cast<messages::UserInterfaceReply*>(reply->msg);
        std::cout << "Server reply: "
                  << "\n    ok? : " << (msg->ok() ? "YES" : "NO")
                  << "\nmessage : " << msg->msg()
                  << "\n";
    }
}

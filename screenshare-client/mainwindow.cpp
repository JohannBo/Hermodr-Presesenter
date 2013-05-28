#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialoglogin.h"

#include <QThread>
#include <QScreen>
#include <QPixmap>
#include <QBuffer>
#include <QCursor>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    DialogLogin l;
//    int login = l.exec();
    xratio = 1.0;
    yratio = 1.0;

    wsSocket = new QWsSocket( this );
    socketStateChanged( wsSocket->state() );

    QObject::connect( wsSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)) );
    QObject::connect( wsSocket, SIGNAL(frameReceived(QString)), this, SLOT(displayMessage(QString)) );
    QObject::connect( wsSocket, SIGNAL(connected()), this, SLOT(socketConnected()) );
    QObject::connect( wsSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
    connectSocket();

}

MainWindow::~MainWindow()
{
    delete wsSocket;
    delete screenshotTimer;
    delete ui;
}

void MainWindow::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    switch ( socketState )
    {
        case QAbstractSocket::UnconnectedState:
            ui->label_WSStatus->setText(tr("Unconnected"));
            break;
        case QAbstractSocket::HostLookupState:
            ui->label_WSStatus->setText(tr("HostLookup"));
            break;
        case QAbstractSocket::ConnectingState:
            ui->label_WSStatus->setText(tr("Connecting"));
            break;
        case QAbstractSocket::ConnectedState:
            ui->label_WSStatus->setText("Connected");
            break;
        case QAbstractSocket::BoundState:
            ui->label_WSStatus->setText(tr("Bound"));
            break;
        case QAbstractSocket::ClosingState:
            ui->label_WSStatus->setText(tr("Closing"));
            break;
        case QAbstractSocket::ListeningState:
            ui->label_WSStatus->setText(tr("Listening"));
            break;
        default:
            ui->label_WSStatus->setText(tr("Unknown"));
            break;
    }
}

void MainWindow::displayMessage(QString message)
{
    qDebug() << "message: " << message;
}

void MainWindow::socketConnected()
{
    qDebug() << "socket connected.";
}

void MainWindow::socketDisconnected()
{
    qDebug() << "socket disconnected.";
}

void MainWindow::connectSocket()
{
    QUrl url("ws://127.0.0.1:9000/presenter?presenter=presenter1");
    wsSocket->connectToHost(url);
}

void MainWindow::on_pushButton_newScreenshare_start_clicked()
{
    screenshotTimer = new QTimer(this);
    connect(screenshotTimer, SIGNAL(timeout()), this, SLOT(sendScreenshot()));
    screenshotTimer->   start(500);

    cursorTimer = new QTimer(this);
    connect(cursorTimer, SIGNAL(timeout()), this, SLOT(sendCursorPosition()));
    cursorTimer->start(200);

}

void MainWindow::sendScreenshot()
{
    QPixmap pixmap;

    QScreen *screen = QGuiApplication::primaryScreen();

    pixmap = screen->grabWindow(0, 0, 0, 1920, 1080);

    int xold = pixmap.width();
    int yold = pixmap.height();

    pixmap = pixmap.scaledToHeight(720,Qt::SmoothTransformation);

    xratio = pixmap.width() / (double)xold;
    yratio = pixmap.height() / (double)yold;

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPG");

    QString base64String = byteArray.toBase64();

    QString part, sendval;
    int frame = 1000;

    while (base64String.size() > 0) {
        if (base64String.size() > frame) {
            part = base64String.right(frame);
            base64String.chop(frame);

            sendval = "{\"type\" : \"image\",\"last\" : \"0\", \"data\" : \"" + part + "\"}";
        } else {
            part = base64String.right(base64String.size());
            base64String.chop(base64String.size());

            sendval = "{\"type\" : \"image\",\"last\" : \"1\", \"data\" : \"" + part + "\"}";
        }
        wsSocket->write(sendval);
    }
}

void MainWindow::sendCursorPosition()
{
    double x = (double)QCursor::pos().x() * xratio;
    double y = (double)QCursor::pos().y() * yratio;
    QString sendval ="{\"type\" : \"cursor\",\"x\" : \"" + QString::number((int)x) + "\",\"y\" : \"" + QString::number((int)y) + "\"}";
    wsSocket->write(sendval);

}

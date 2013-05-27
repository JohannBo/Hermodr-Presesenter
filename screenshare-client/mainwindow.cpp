#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialoglogin.h"

#include <QThread>
#include <QScreen>
#include <QPixmap>
#include <QBuffer>
#include <QFile>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    DialogLogin l;
//    int login = l.exec();

    wsSocket = new QWsSocket( this );
    socketStateChanged( wsSocket->state() );

    QObject::connect( wsSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)) );
    QObject::connect( wsSocket, SIGNAL(frameReceived(QString)), this, SLOT(displayMessage(QString)) );
    QObject::connect( wsSocket, SIGNAL(connected()), this, SLOT(socketConnected()) );
    QObject::connect( wsSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
    connectSocket();
    imagenumber = 1;

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
    QTimer *screenshotTimer = new QTimer(this);
    connect(screenshotTimer, SIGNAL(timeout()), this, SLOT(takeScreenshot()));
    screenshotTimer->start(200);

}

void MainWindow::takeScreenshot()
{
    QPixmap pixmap;

    QScreen *screen = QGuiApplication::primaryScreen();

    pixmap = screen->grabWindow(0, 0, 0, 1920, 1080);

    pixmap = pixmap.scaledToHeight(720,Qt::SmoothTransformation);

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

            sendval = "{\"last\" : \"0\", \"data\" : \"" + part + "\"}";
        } else {
            part = base64String.right(base64String.size());
            base64String.chop(base64String.size());

            sendval = "{\"last\" : \"1\", \"data\" : \"" + part + "\"}";
        }
        wsSocket->write(sendval);
    }
    QString filename = "myfile_" + QString::number(imagenumber) + ".jpg";

    imagenumber++;
    QFile myFile(filename);
    myFile.open(QIODevice::WriteOnly);
    pixmap.save(&myFile, "JPG");
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialoglogin.h"

#include <QThread>
#include <QScreen>
#include <QPixmap>
#include <QBuffer>
#include <QCursor>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

const int MainWindow::FRAME_LENGTH = 1000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    DialogLogin l;
//    int login = l.exec();
    xratio = 1.0;
    yratio = 1.0;

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            images[i][j] = "";
        }
    }
    oldCursorX = 0;
    oldCursorY = 0;

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
    screenshotTimer->start(500);

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

    pixmap = pixmap.scaledToHeight(720, Qt::SmoothTransformation);

    xratio = pixmap.width() / (double)xold;
    yratio = pixmap.height() / (double)yold;

    splitImage(pixmap);
}

void MainWindow::sendCursorPosition()
{
    int x = (double)QCursor::pos().x() * xratio;
    int y = (double)QCursor::pos().y() * yratio;
    if (x != oldCursorX || y != oldCursorY) {
        QString sendval ="{\"type\" : \"cursor\",\"x\" : \"" + QString::number(x) + "\",\"y\" : \"" + QString::number(y) + "\"}";
        wsSocket->write(sendval);
        oldCursorX = x;
        oldCursorY = y;
    }
}

void MainWindow::splitImage(QPixmap image){
    int partX = image.width() / GRID_SIZE;
    int partY = image.height() / GRID_SIZE;
    int posX = 0;
    int posY = 0;


    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            QPixmap imagePart = image.copy(posX, posY, partX, partY);
            posX += partX;

            QString oldPart = images[x][y];

            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            imagePart.save(&buffer, "JPG");
            QString newPart = byteArray.toBase64();

            if (newPart != oldPart) {
                images[x][y] = newPart;
                sendImage(newPart, x, y);

//                qDebug() << "X: " << posX << " Y: " << posY << " width: " << imagePart.width() << " heigth: " << imagePart.height() << " length: " << newPart.length();
            }
        }
        posY += partY;
        posX = 0;
    }


}

void MainWindow::sendImage(QString imagePart, int posX, int posY) {

    while (imagePart.length() > 0) {
        QString type = "image";
        QString last = "0";
        QString partToSend = "";
        QString x = QString::number(posX);
        QString y = QString::number(posY);

        if (imagePart.length() > FRAME_LENGTH) {
            partToSend = imagePart.right(FRAME_LENGTH);
            imagePart.chop(FRAME_LENGTH);
        } else {
            partToSend = imagePart;
            imagePart = "";
            last = "1";
        }

        QString sendVal = "{"
                "\"type\":\"" + type + "\","
                "\"last\":\"" + last + "\","
                "\"x\":\"" + x + "\","
                "\"y\":\"" + y + "\","
                "\"data\":\"" + partToSend + "\""
                "}";
        wsSocket->write(sendVal);
    }
}














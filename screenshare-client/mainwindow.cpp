#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialoglogin.h"

#include <QScreen>
#include <QBuffer>
#include <QCursor>
#include <QFile>
#include <QPainter>
#include <QMessageBox>
#include <QProcess>



const int MainWindow::FRAME_LENGTH = 1000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    screenshotTimer = new QTimer(this);
    isScreenshotTimer = true;
    cursorTimer = new QTimer(this);
    isCursorTimer = true;
    audioTimer = new QTimer(this);
    isAudioTimer = true;

    //    DialogLogin l;
    //    int login = l.exec();

}

MainWindow::~MainWindow()
{
    if (isAudioRecorderOffline) {
        delete audioRecorderOffline;
    }
    if (isAudioRecorderOnline) {
        delete audioRecorderOnline;
    }
    if (isAudioTimer) {
        delete audioTimer;
    }
    if (isCursorTimer) {
        delete cursorTimer;
    }
    if (isScreenshotTimer) {
        delete screenshotTimer;
    }
    if (isWsSocket) {
        delete wsSocket;
    }

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

void MainWindow::displayMessage(const QString message)
{
    qDebug() << "message: " << message;
}

void MainWindow::socketConnected()
{
    qDebug() << "socket connected.";
    startScreenshare();

}

void MainWindow::socketDisconnected()
{
    qDebug() << "socket disconnected.";
}

void MainWindow::connectSocket()
{

    QUrl url("ws://" + ui->lineEdit_settings_serverUrl->text() + "/presenter?presenter=" + ui->lineEdit_settings_userName->text());
    wsSocket->connectToHost(url);
}

void MainWindow::startScreenshare()
{
    // todo schöner
    if (ui->checkBox_newScreenshare_live->isChecked()) {
        if (ui->checkBox_newScreenshare_live_audio->isChecked()) {
            connect(audioTimer, SIGNAL(timeout()), this, SLOT(stopRecorderOnline()));
            startRecorderOnline();
        }
    }

    if (ui->checkBox_newScreenshare_file->isChecked()) {
        if (ui->checkBox_newScreenshare_file_audio->isChecked()) {
            startRecorderOffline();
        }
    }
    connect(screenshotTimer, SIGNAL(timeout()), this, SLOT(takeScreenshot()));
    screenshotTimer->start(500);

    connect(cursorTimer, SIGNAL(timeout()), this, SLOT(sendCursorPosition()));
    cursorTimer->start(200);




}

void MainWindow::takeScreenshot()
{
    QPixmap screenshot;

    QScreen *screen = QGuiApplication::primaryScreen();

    // todo werte aus settings
    int posX = ui->lineEdit_settings_positionX->text().toInt();
    int posY = ui->lineEdit_settings_positionY->text().toInt();
    int screenshotHeight = ui->lineEdit_settings_height->text().toInt();
    int screenshotWidth = ui->lineEdit_settings_width->text().toInt();
    int targetHeight = ui->lineEdit_settings_targetHeight->text().toInt();

    screenshot = screen->grabWindow(0, posX, posY, screenshotWidth, screenshotHeight);

    int xold = screenshot.width();
    int yold = screenshot.height();

    screenshot = screenshot.scaledToHeight(targetHeight, Qt::SmoothTransformation);

    xratio = screenshot.width() / (double)xold;
    yratio = screenshot.height() / (double)yold;

    if (ui->checkBox_newScreenshare_file->isChecked()) {
        saveScreenshot(screenshot);
    }
    if (ui->checkBox_newScreenshare_live->isChecked()) {
        sendScreenshot(screenshot);
    }


}

void MainWindow::saveScreenshot(QPixmap screenshot)
{
    // draw mouse into screenshot
    QPainter qp(&screenshot);
    qp.fillRect(oldCursorX, oldCursorY, 5, 5, Qt::black);
    qp.fillRect(oldCursorX+1, oldCursorY+1, 3, 3, Qt::white);
    qp.end();

    // save screenshot to file
    QString filename = "pic" + QString::number(pictureFileIndex++) + ".jpg";
    QFile myFile(filename);
    myFile.open(QIODevice::WriteOnly);
    screenshot.save(&myFile, "JPG");
}

void MainWindow::sendScreenshot(const QPixmap screenshot)
{
    int partX = screenshot.width() / GRID_SIZE;
    int partY = screenshot.height() / GRID_SIZE;
    int posX = 0;
    int posY = 0;


    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            QPixmap imagePart = screenshot.copy(posX, posY, partX, partY);
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
            }
        }
        posY += partY;
        posX = 0;
    }
}

void MainWindow::sendCursorPosition()
{
    int newCursorX = (QCursor::pos().x() - ui->lineEdit_settings_positionX->text().toInt()) * xratio;
    int newCursorY = (QCursor::pos().y() - ui->lineEdit_settings_positionY->text().toInt() ) * yratio;
    if (newCursorX != oldCursorX || newCursorY != oldCursorY) {
        QString type = "cursor";
        QString x = QString::number(newCursorX);
        QString y = QString::number(newCursorY);

        QString sendVal = "{"
                "\"type\":\"" + type + "\","
                "\"x\":\"" + x + "\","
                "\"y\":\"" + y + "\""
                "}";

        if (ui->checkBox_newScreenshare_live->isChecked()) {
            wsSocket->write(sendVal);
        }
        oldCursorX = newCursorX;
        oldCursorY = newCursorY;
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

void MainWindow::startRecorderOffline()
{
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/vorbis");
    audioSettings.setQuality(QMultimedia::NormalQuality);

    audioRecorderOffline= new QAudioRecorder;
    isAudioRecorderOffline = true;
    audioRecorderOffline->setEncodingSettings(audioSettings);

    audioRecorderOffline->setOutputLocation(QUrl::fromLocalFile("sound.ogg"));

    audioRecorderOffline->record();
}

void MainWindow::startRecorderOnline()
{
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/vorbis");
    audioSettings.setQuality(QMultimedia::VeryLowQuality);

    audioRecorderOnline = new QAudioRecorder;
    isAudioRecorderOnline = true;
    audioRecorderOnline->setEncodingSettings(audioSettings);

    audioRecorderOnline->setOutputLocation(QUrl::fromLocalFile("sound" + QString::number(soundFileIndex++) + ".ogg"));

//    audioTimer->singleShot(1000, this, SLOT(stopRecorderOnline()));
    audioTimer->start(1000);
    audioRecorderOnline->record();

}

void MainWindow::stopRecorderOnline()
{
    audioTimer->stop();
    audioRecorderOnline->stop();
    QUrl soundUrl = audioRecorderOnline->outputLocation();
    delete audioRecorderOnline;
    isAudioRecorderOnline = false;
    QString sound = openSoundFile(soundUrl);
    sendAudio(sound);
    startRecorderOnline();

}

QString MainWindow::openSoundFile(QUrl url)
{
    QFile myFile(url.url());
    myFile.open(QIODevice::ReadOnly);
    QByteArray array = myFile.readAll();
    myFile.close();
    myFile.remove();
    return array.toBase64();
}

void MainWindow::sendAudio(QString audio)
{
    while (audio.length() > 0) {
        QString type = "audio";
        QString last = "0";
        QString partToSend = "";

        if (audio.length() > FRAME_LENGTH) {
            partToSend = audio.right(FRAME_LENGTH);
            audio.chop(FRAME_LENGTH);
        } else {
            partToSend = audio;
            audio = "";
            last = "1";
        }

        QString sendVal = "{"
                "\"type\":\"" + type + "\","
                "\"last\":\"" + last + "\","
                "\"data\":\"" + partToSend + "\""
                "}";
        wsSocket->write(sendVal);
    }
}

void MainWindow::on_pushButton_newScreenshare_start_clicked()
{
    if (!ui->checkBox_newScreenshare_file->isChecked() && !ui->checkBox_newScreenshare_live->isChecked()) {
        QMessageBox msgBox(this);
        msgBox.setText("No Target has been chosen.");
        msgBox.exec();
        return;
    }

    ui->pushButton_newScreenshare_start->setEnabled(false);
    ui->pushButton_newScreenshare_stop->setEnabled(true);
    ui->pushButton_newScreenshare_pause->setEnabled(true);
    ui->tab_settings->setEnabled(false);

    xratio = 1.0;
    yratio = 1.0;
    oldCursorX = 0;
    oldCursorY = 0;
    pictureFileIndex = 0;
    soundFileIndex = 0;

    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            images[x][y] = "";
        }
    }

    // Todo directory anlegen

    if (ui->checkBox_newScreenshare_live->isChecked()) {
        wsSocket = new QWsSocket( this );
        isWsSocket = true;
        socketStateChanged( wsSocket->state() );

        connect( wsSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)) );
        connect( wsSocket, SIGNAL(frameReceived(QString)), this, SLOT(displayMessage(QString)) );
        connect( wsSocket, SIGNAL(connected()), this, SLOT(socketConnected()) );
        connect( wsSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
        connectSocket();
    } else {
        startScreenshare();
    }


}

void MainWindow::on_pushButton_newScreenshare_stop_clicked()
{
    QString program = ui->lineEdit_settings_ffmpeg->text();
    QStringList arguments;
    arguments << "-i" << "sound0.ogg" << "blubb.mp3";
    QProcess *myProcess = new QProcess(this);
    myProcess->start(program, arguments);

    // todo ffmpeg

    if (isCursorTimer) {
        cursorTimer->stop();
    }

    if (isScreenshotTimer) {
        screenshotTimer->stop();
    }

    if (isAudioTimer) {
        audioTimer->stop();
    }

    if (isAudioRecorderOffline) {
        audioRecorderOffline->stop();
    }

    if (isAudioRecorderOnline) {
        audioRecorderOnline->stop();
    }

    if (isWsSocket) {
        wsSocket->abort("disconnect");
    }


    ui->pushButton_newScreenshare_start->setEnabled(true);
    ui->pushButton_newScreenshare_stop->setEnabled(false);
    ui->pushButton_newScreenshare_pause->setEnabled(false);
    ui->tab_settings->setEnabled(true);
}

void MainWindow::on_checkBox_newScreenshare_live_clicked()
{
    if (ui->checkBox_newScreenshare_live->isChecked()) {
        ui->checkBox_newScreenshare_live_audio->setEnabled(true);
    } else {
        ui->checkBox_newScreenshare_live_audio->setEnabled(false);
    }
}

void MainWindow::on_checkBox_newScreenshare_file_clicked()
{
    if (ui->checkBox_newScreenshare_file->isChecked()) {
        ui->checkBox_newScreenshare_file_audio->setEnabled(true);
    } else {
        ui->checkBox_newScreenshare_file_audio->setEnabled(false);
    }
}

void MainWindow::on_pushButton_newScreenshare_pause_clicked()
{
    qDebug() << "todo";
}

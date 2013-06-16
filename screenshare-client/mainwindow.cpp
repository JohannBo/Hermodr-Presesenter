/*
 * Copyright (C) 2013
 * johann.bornholdt@gmail.com
 *
 * This file is part of Hermodr-Presenter.
 *
 * Hermodr-Presenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hermodr-Presenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hermodr-Presenter.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScreen>
#include <QBuffer>
#include <QCursor>
#include <QFile>
#include <QDir>
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>

const int MainWindow::FRAME_LENGTH = 1000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isWsSocket = false;
    isScreenshotTimer = false;
    isCursorTimer = false;
    isAudioTimer = false;
    isffmpeg = false;
#ifdef Q_OS_WIN
    ui->lineEdit_settings_dshow->setText("Mikrofon (Realtek High Definiti");
    ui->lineEdit_settings_ffmpeg->setText("C:\\ffmpeg\\bin\\ffmpeg");
#endif


}

MainWindow::~MainWindow()
{
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

    if (ui->checkBox_newScreenshare_live->isChecked() && ui->checkBox_newScreenshare_live_audio->isChecked()) {

        startRecorderOffline();

        audioTimer = new QTimer(this);
        isAudioTimer = true;
        connect(audioTimer,SIGNAL(timeout()), this, SLOT(startRecorderOnline()));
        audioTimer->start(1000);

    } else if (ui->checkBox_newScreenshare_file->isChecked() && ui->checkBox_newScreenshare_file_audio->isChecked()) {
        startRecorderOffline();
    }



    screenshotTimer = new QTimer(this);
    isScreenshotTimer = true;
    connect(screenshotTimer, SIGNAL(timeout()), this, SLOT(takeScreenshot()));
    screenshotTimer->start(500);

    cursorTimer = new QTimer(this);
    isCursorTimer = true;
    connect(cursorTimer, SIGNAL(timeout()), this, SLOT(sendCursorPosition()));
    cursorTimer->start(50);




}

void MainWindow::takeScreenshot()
{
    QPixmap screenshot;

    QScreen *screen = QGuiApplication::primaryScreen();

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
    qp.fillRect(oldCursorX + 1, oldCursorY + 1, 3, 3, Qt::white);
    qp.end();

    // save screenshot to file
    QString filename = "frame" + QString::number(pictureFileIndex++) + ".jpg";
    QFile myFile(ui->lineEdit_newScreenshare_title->text() + "/" + filename);
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
    QString program = ui->lineEdit_settings_ffmpeg->text();
    QString filename = ui->lineEdit_newScreenshare_title->text() + "/sound.wav";

    QFile file(filename);
    if(file.exists()) {
        file.remove();
    }

    QStringList arguments;
    //ffmpeg -f alsa -ac 1 -i pulse -b:a 32k -codec:a libvorbis sound.ogg
//    arguments << "-f" << "alsa" << "-ac" << "1" << "-i" << "pulse" << "-b:a" << "32k" << "-codec:a" << "libvorbis" << filename;

#ifdef Q_OS_WIN
    arguments << "-f" << "dshow" << "-i" << ("audio=" + ui->lineEdit_settings_dshow->text() + "") << "-codec:a" << "pcm_s16le" << filename;
#else
    arguments << "-f" << "alsa" << "-ac" << "1" << "-i" << "pulse" << "-codec:a" << "pcm_s16le" << filename;
#endif

    ffmpeg = new QProcess(this);
    isffmpeg = true;
    ffmpeg->start(program, arguments);
}

void MainWindow::startRecorderOnline()
{
    if (secondInAudioFile < 0) {
        secondInAudioFile++;
        return;
    }

    int thisSecond = secondInAudioFile++;
    QString filenameOut = ui->lineEdit_newScreenshare_title->text() + "/test" + QString::number(thisSecond) +".ogg";
    QString filenameIn = ui->lineEdit_newScreenshare_title->text() + "/sound.wav";

    QString program = ui->lineEdit_settings_ffmpeg->text();
    QStringList arguments;

    arguments << "-i" << filenameIn << "-ss" << QString::number(thisSecond) << "-t" << "1" << "-b:a" << "64k" << "-codec:a" << "libvorbis" << filenameOut;
    //ffmpeg -i test.ogg -ss 1 -t 1 -b:a 32k -codec:a libvorbis footest.ogg

    QProcess ffmpegChunk(this);
    ffmpegChunk.start(program, arguments);

    if (!ffmpegChunk.waitForFinished()){
        qDebug() << "error waiting for process";
    }
    QFile myFile(filenameOut);
    myFile.open(QIODevice::ReadOnly);
    QByteArray array = myFile.readAll();
    myFile.close();
    myFile.remove();
    sendAudio( array.toBase64());


    // eine sekunde aufnehmen
    // ffmpeg -f alsa -ac 1 -i pulse -t 1 -b:a 32k -codec:a libvorbis test.ogg

    // aufnahme starten
    // ffmpeg -f alsa -ac 1 -i pulse -b:a 32k -codec:a libvorbis test.ogg

    // chunk aus test.ogg an stelle ss für t sekunden nach footest speichern
    // ffmpeg -i test.ogg -ss 1 -t 1 -b:a 32k -codec:a libvorbis footest.ogg

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
    ui->lineEdit_newScreenshare_title->setEnabled(false);
    ui->plainTextEdit_newScreenshare_description->setEnabled(false);

    xratio = 1.0;
    yratio = 1.0;
    oldCursorX = 0;
    oldCursorY = 0;
    pictureFileIndex = 0;
    soundFileIndex = 0;
    secondInAudioFile = -2;

    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            images[x][y] = "";
        }
    }

    QDir *qDir = new QDir();
    qDir->mkdir(ui->lineEdit_newScreenshare_title->text());
    delete qDir;

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
    if (isCursorTimer) {
        cursorTimer->stop();
        delete cursorTimer;
        isCursorTimer = false;
    }

    if (isScreenshotTimer) {
        screenshotTimer->stop();
        delete screenshotTimer;
        isScreenshotTimer = false;
    }

    if (isAudioTimer) {
        audioTimer->stop();
        delete audioTimer;
        isAudioTimer = false;
    }

    if (isffmpeg) {
        ffmpeg->kill();
        // todo save file
        delete ffmpeg;
        isffmpeg = false;
    }

    if (isWsSocket) {
        wsSocket->abort("disconnect");
        delete wsSocket;
        isWsSocket = false;
    }

    if (ui->checkBox_newScreenshare_file->isChecked()) {
        //        QString filenameOut = ui->lineEdit_newScreenshare_title->text() + ".webm";
        QString filenameOut = QFileDialog::getSaveFileName(this, tr("Save File"), QString(), tr("Webm Videos (*.webm)"));


        if (!filenameOut.isEmpty()) {
            if (!filenameOut.endsWith(".webm")) {
                filenameOut.append(".webm");
            }
            QFile file(filenameOut);
            if (file.exists()) {
                file.remove();
            }

            QString program = ui->lineEdit_settings_ffmpeg->text();
            QStringList arguments;

            if (ui->checkBox_newScreenshare_file_audio->isChecked()) {
                QString filenameSoundIn = ui->lineEdit_newScreenshare_title->text() + "/sound.wav";
                // ffmpeg -i sound0.ogg -f image2 -framerate 2 -i frame%d.jpg c.webm
                arguments << "-i" << filenameSoundIn << "-f" << "image2" << "-framerate" << "2" << "-i" << ui->lineEdit_newScreenshare_title->text() + "/frame%d.jpg" << filenameOut;
            } else {
                // ffmpeg -f image2 -framerate 2 -i frame%d.jpg c.webm
                arguments << "-f" << "image2" << "-framerate" << "2" << "-i" << ui->lineEdit_newScreenshare_title->text() + "/frame%d.jpg" << filenameOut;
            }

            QProcess ffmpegVideoCreator(this);
            ffmpegVideoCreator.start(program, arguments);

            if (!ffmpegVideoCreator.waitForFinished()){
                qDebug() << "error waiting for process";
            }

            QDir dir(ui->lineEdit_newScreenshare_title->text());
            dir.removeRecursively();

        } else {
            qDebug() << "saving file canceled";
        }
    } else {
        QDir dir(ui->lineEdit_newScreenshare_title->text());
        dir.removeRecursively();
    }

    ui->pushButton_newScreenshare_start->setEnabled(true);
    ui->pushButton_newScreenshare_stop->setEnabled(false);
    ui->pushButton_newScreenshare_pause->setEnabled(false);
    ui->tab_settings->setEnabled(true);
    ui->lineEdit_newScreenshare_title->setEnabled(true);
    ui->plainTextEdit_newScreenshare_description->setEnabled(true);
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

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox mbox(this);
    mbox.setText("Digital Whiteboard. By Johann Bornholdt\njohann.bornholdt@gmail.com");
    mbox.exec();

}

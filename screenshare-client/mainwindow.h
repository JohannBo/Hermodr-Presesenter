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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QUrl>
#include <QProcess>

#include "QWsSocket.h"

#define GRID_SIZE 20


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void displayMessage(const QString);
    void socketConnected();
    void socketDisconnected();
    void connectSocket();
    
private slots:
    void on_pushButton_newScreenshare_start_clicked();
    void startScreenshare();
    void takeScreenshot();
    void sendScreenshot(const QPixmap screenshot);
    void saveScreenshot(QPixmap screenshot);
    void sendCursorPosition();
    void sendImage(QString imagePart, int posX, int posY);

    void startRecorderOnline();

    void startRecorderOffline();

    void sendAudio(QString audio);

    void on_pushButton_newScreenshare_stop_clicked();

    void on_checkBox_newScreenshare_live_clicked();

    void on_checkBox_newScreenshare_file_clicked();

    void on_pushButton_newScreenshare_pause_clicked();

    void on_actionQuit_triggered();

    void on_actionAbout_triggered();

protected:


private:
    static const int FRAME_LENGTH;
    Ui::MainWindow *ui;
    QWsSocket *wsSocket;
    bool isWsSocket;
    QTimer *screenshotTimer;
    bool isScreenshotTimer;
    QTimer *cursorTimer;
    bool isCursorTimer;
    QTimer *audioTimer;
    bool isAudioTimer;
    QProcess *ffmpeg;
    bool isffmpeg;
    double xratio;
    double yratio;
    QString images[GRID_SIZE][GRID_SIZE];
    int oldCursorX;
    int oldCursorY;
    int pictureFileIndex;
    int soundFileIndex;
    int secondInAudioFile;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QUrl>
#include <QAudioRecorder>

#include "QWsSocket.h"
#include "dialoglogin.h"

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
    void stopRecorderOnline();
    void startRecorderOffline();
    QString openSoundFile(QUrl url);
    void sendAudio(QString audio);

    void on_pushButton_newScreenshare_stop_clicked();

    void on_checkBox_newScreenshare_live_clicked();

    void on_checkBox_newScreenshare_file_clicked();

    void on_pushButton_newScreenshare_pause_clicked();

protected:


private:
    static const int FRAME_LENGTH;
    Ui::MainWindow *ui;
    QWsSocket *wsSocket;
    bool isWsSocket;
    QAudioRecorder *audioRecorderOnline;
    bool isAudioRecorderOnline;
    QAudioRecorder *audioRecorderOffline;
    bool isAudioRecorderOffline;
    QTimer *screenshotTimer;
    bool isScreenshotTimer;
    QTimer *cursorTimer;
    bool isCursorTimer;
    QTimer *audioTimer;
    bool isAudioTimer;
    double xratio;
    double yratio;
    QString images[GRID_SIZE][GRID_SIZE];
    int oldCursorX;
    int oldCursorY;
    int pictureFileIndex;
    int soundFileIndex;
};

#endif // MAINWINDOW_H

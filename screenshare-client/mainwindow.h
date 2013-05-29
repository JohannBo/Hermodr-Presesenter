#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QPixmap>

#include "QWsSocket.h"
#include "dialoglogin.h"


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
    void displayMessage(QString);
    void socketConnected();
    void socketDisconnected();
    void connectSocket();
    
private slots:
    void on_pushButton_newScreenshare_start_clicked();
    void sendScreenshot();
    void sendCursorPosition();
    void splitImage(QPixmap image);
    void sendImage(QString imagePart, int posX, int posY);

protected:
    QWsSocket *wsSocket;

private:
    Ui::MainWindow *ui;
    QTimer *screenshotTimer;
    QTimer *cursorTimer;
    double xratio;
    double yratio;
    const int frameLength = 1000;
    const int partSize = 20;
    QString images[20][20];
    int oldCursorX;
    int oldCursorY;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>

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
    void takeScreenshot();

protected:
    QWsSocket *wsSocket;

private:
    Ui::MainWindow *ui;
    QTimer *screenshotTimer;
    int imagenumber;
};

#endif // MAINWINDOW_H

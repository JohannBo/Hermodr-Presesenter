#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

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
    
private slots:
    void on_pushButton_newScreenshare_start_clicked();

protected:
    QWsSocket *wsSocket;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

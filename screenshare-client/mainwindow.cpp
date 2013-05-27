#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialoglogin.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DialogLogin l;
    int login = l.exec();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_newScreenshare_start_clicked()
{

}

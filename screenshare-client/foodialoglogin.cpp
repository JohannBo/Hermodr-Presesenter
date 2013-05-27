#include "dialoglogin.h"
#include "ui_dialoglogin.h"
#include "mainwindow.h"

DialogLogin::DialogLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogin)
{
    ui->setupUi(this);
}

DialogLogin::~DialogLogin()
{
    delete ui;
}

void DialogLogin::on_pushButton_quit_clicked()
{
    qApp->quit();
}

void DialogLogin::on_pushButton_login_clicked()
{
//    MainWindow mw;

//    mw.show();
}

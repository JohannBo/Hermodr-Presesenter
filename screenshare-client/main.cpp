#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    DialogLogin l;
//    l.show();


    MainWindow w;
    w.show();
    
    return a.exec();
}

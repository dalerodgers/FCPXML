#include "mainwindow.h"
#include <QApplication>
#include "trimmer.h"
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    TrimmerWorker tw;

    QObject::connect(&w, SIGNAL(trimStart(QString,QString,bool,QString)), &tw, SLOT(go(QString,QString,bool,QString)));
    QObject::connect(&tw, SIGNAL(allDone()), &w, SLOT(trimFinished()));

    QObject::connect(&tw, SIGNAL(debug(QString)), &w, SLOT(debug(QString)));

    w.show();

    return a.exec();
}

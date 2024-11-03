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

    QObject::connect(&tw, SIGNAL(debug(QString)), &w, SLOT(debug(QString)) );//, Qt::BlockingQueuedConnection);
    QObject::connect(&tw, SIGNAL(cout(QString)), &w, SLOT(cout(QString)) );//, Qt::BlockingQueuedConnection);
    QObject::connect(&tw, SIGNAL(cerr(QString)), &w, SLOT(cerr(QString)) );//, Qt::BlockingQueuedConnection);

    w.show();

    return a.exec();
}

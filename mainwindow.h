#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QXmlStreamReader>
#include <QMap>
#include <QString>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void chooseEDF();
    void setOutputFolder();
    void go();

signals:
   void trimStart(QString outputFolder, QString edfFile, bool stripAudio, QString prefix);

public slots:
   void trimFinished();
   void debug(QString str);
};

#endif // MAINWINDOW_H

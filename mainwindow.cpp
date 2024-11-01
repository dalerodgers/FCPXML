#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QThread>
#include <string.h>
#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stripAudio->setChecked(true);

    connect(ui->buttonChooseEDF, SIGNAL(clicked()), this, SLOT(chooseEDF()) );
    connect(ui->buttonSetOutputFolder, SIGNAL(clicked()), this, SLOT(setOutputFolder()) );
    connect(ui->buttonGo, SIGNAL(clicked()), this, SLOT(go())  );
}

void MainWindow::chooseEDF()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select FCPXML Cutlist"),
                                                    "",
                                                    tr("FCPXML cutlist (*.fcpxml);;All files (*.*)"));

    if( !fileName.isEmpty() )
    {
        ui->pathEdfFile->setText(fileName);
    }
}

void MainWindow::setOutputFolder()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Select Output Folder"),
                                                        "");

    if( !dirName.isEmpty() )
    {
        ui->pathOutputFolder->setText(dirName);
    }
}

void MainWindow::go()
{
    ui->buttonGo->setEnabled(false);
    emit trimStart(ui->pathOutputFolder->text(), ui->pathEdfFile->text(), ui->stripAudio->isChecked(), ui->prefix->text());
}

void MainWindow::trimFinished()
{
    ui->buttonGo->setEnabled(true);
}

void MainWindow::debug(QString str)
{
    ui->debug->appendPlainText(str);
}

MainWindow::~MainWindow()
{
    delete ui;
}

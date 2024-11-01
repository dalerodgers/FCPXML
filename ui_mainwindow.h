/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *buttonChooseEDF;
    QPushButton *buttonSetOutputFolder;
    QLineEdit *pathEdfFile;
    QLineEdit *pathOutputFolder;
    QPushButton *buttonGo;
    QPlainTextEdit *debug;
    QLineEdit *prefix;
    QRadioButton *stripAudio;
    QLabel *label;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(773, 549);
        MainWindow->setMinimumSize(QSize(773, 549));
        MainWindow->setMaximumSize(QSize(773, 549));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        buttonChooseEDF = new QPushButton(centralWidget);
        buttonChooseEDF->setObjectName(QStringLiteral("buttonChooseEDF"));
        buttonChooseEDF->setGeometry(QRect(10, 490, 141, 51));
        buttonSetOutputFolder = new QPushButton(centralWidget);
        buttonSetOutputFolder->setObjectName(QStringLiteral("buttonSetOutputFolder"));
        buttonSetOutputFolder->setGeometry(QRect(10, 430, 141, 51));
        pathEdfFile = new QLineEdit(centralWidget);
        pathEdfFile->setObjectName(QStringLiteral("pathEdfFile"));
        pathEdfFile->setGeometry(QRect(160, 490, 601, 51));
        pathEdfFile->setReadOnly(true);
        pathOutputFolder = new QLineEdit(centralWidget);
        pathOutputFolder->setObjectName(QStringLiteral("pathOutputFolder"));
        pathOutputFolder->setGeometry(QRect(160, 430, 601, 51));
        pathOutputFolder->setReadOnly(true);
        buttonGo = new QPushButton(centralWidget);
        buttonGo->setObjectName(QStringLiteral("buttonGo"));
        buttonGo->setGeometry(QRect(10, 370, 141, 51));
        debug = new QPlainTextEdit(centralWidget);
        debug->setObjectName(QStringLiteral("debug"));
        debug->setGeometry(QRect(10, 10, 751, 351));
        debug->setReadOnly(true);
        prefix = new QLineEdit(centralWidget);
        prefix->setObjectName(QStringLiteral("prefix"));
        prefix->setGeometry(QRect(410, 370, 351, 51));
        prefix->setReadOnly(false);
        stripAudio = new QRadioButton(centralWidget);
        stripAudio->setObjectName(QStringLiteral("stripAudio"));
        stripAudio->setGeometry(QRect(190, 380, 89, 20));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(360, 390, 47, 13));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        buttonChooseEDF->setText(QApplication::translate("MainWindow", "Choose FCPXML file", Q_NULLPTR));
        buttonSetOutputFolder->setText(QApplication::translate("MainWindow", "Set Output Folder", Q_NULLPTR));
        buttonGo->setText(QApplication::translate("MainWindow", "Go", Q_NULLPTR));
        stripAudio->setText(QApplication::translate("MainWindow", "Strip audio", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindow", "Prefix:", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

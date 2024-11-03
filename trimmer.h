#ifndef TRIMMER_H
#define TRIMMER_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QMap>
#include <QThread>
#include <QProcess>

class TrimmerWorker : public QObject
{
    Q_OBJECT

public:
    TrimmerWorker( );
    ~TrimmerWorker();

private:
    struct Asset
    {
        QString src;
        QString id;
        QString name;
    };

    QThread thread_;

    QString fcpxmlFile_;
    QString outputFolder_;
    bool stripAudio_;
    QString prefix_;
    int errorCount_;

    QMap<QString, Asset> assetMap;

    void parse_fcpxml(QXmlStreamReader& xmlStream);
    void parse_resources(QXmlStreamReader& xmlStream);
    void parse_asset(QXmlStreamReader& xmlStream);
    void parse_library(QXmlStreamReader& xmlStream);
    void parse_event(QXmlStreamReader& xmlStream);
    void parse_project(QXmlStreamReader& xmlStream);
    void parse_sequence(QXmlStreamReader& xmlStream);
    void parse_spine(QXmlStreamReader& xmlStream);
    void parse_asset_clip(QXmlStreamReader& xmlStream);

    static bool doesFileExist(const QString& fileName);
    static QString resolveFilename(QString src);
    float extractTime(QString& str);
//    void trim(ClipItem& clipItem);
    void trimAll();

    QProcess process_;

public slots:
    void go(QString outputFolder, QString fcpxmlFile, bool stripAudio, QString prefix);

    void on_readyReadStandardError();
    void on_readyReadStandardOutput();

private slots:
    void threadFunc();

signals:
    void debug(const QString str);
    void cout(const QString str);
    void cerr(const QString str);

    void allDone();
    void gogo();
};

#endif // TRIMMER_H

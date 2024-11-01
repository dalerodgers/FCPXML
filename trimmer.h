#ifndef TRIMMER_H
#define TRIMMER_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QMap>
#include <QThread>

class TrimmerWorker : public QObject
{
    Q_OBJECT

public:
    TrimmerWorker();
    ~TrimmerWorker();

private:
//    struct File
//    {
//        QString id;
//        QString name;
//        QString pathurl;
//        QString timebase;
//    };

//    struct ClipItem
//    {
//       QString name;
//       QString timebase;
//       QString file_in;
//       QString file_out;
//       QString file_subclipinfo_startoffset;
//       QString file_subclipinfo_endoffset;

//       File* filey;
//    };

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
//    void parse_video(QXmlStreamReader& xmlStream);
//    void parse_track(QXmlStreamReader& xmlStream);
//    void parse_clipitem(QXmlStreamReader& xmlStream);
//    void parse_clipitem_rate(QXmlStreamReader& xmlStream, ClipItem& clipItem);
//    void parse_clipitem_file(QXmlStreamReader& xmlStream, ClipItem& clipItem);
//    void parse_clipitem_file_rate(QXmlStreamReader& xmlStream, File& filey);
//    void parse_clipitem_subclipinfo(QXmlStreamReader& xmlStream, ClipItem& clipItem);

    static bool doesFileExist(const QString& fileName);
    static QString resolveFilename(QString src);
    float extractTime(QString& str);
//    void trim(ClipItem& clipItem);
    void trimAll();

public slots:
    void go(QString outputFolder, QString fcpxmlFile, bool stripAudio, QString prefix);

private slots:
    void threadFunc();

signals:
    void debug(const QString str);
    void allDone();
    void gogo();
};

#endif // TRIMMER_H

#include "trimmer.h"

#include <QFileDialog>
#include <QThread>
#include <string.h>
#include <stdio.h>
#include <QEventLoop>

static const QString LightworksMediaPath("C:\\Users\\Public\\Documents\\Lightworks\\Media\\Material\\");

TrimmerWorker::TrimmerWorker()
{
    this->moveToThread(&thread_);
    connect(&thread_, SIGNAL(started()), this, SLOT(threadFunc()));
    thread_.start();

    connect( &process_, SIGNAL(readyReadStandardError()), this, SLOT(on_readyReadStandardError()), Qt::DirectConnection );
    connect( &process_, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readyReadStandardOutput()), Qt::DirectConnection );
}

void TrimmerWorker::threadFunc()
{
    QEventLoop eventLoop;

    connect(this, SIGNAL(gogo()), &eventLoop, SLOT(quit()));

    while(1)
    {
        eventLoop.exec();

        trimAll();
    }
}

void TrimmerWorker::go(QString outputFolder, QString fcpxmlFile, bool stripAudio, QString prefix)
{
    emit debug("");
    errorCount_ = 0;

    outputFolder_ = outputFolder;
    fcpxmlFile_ = fcpxmlFile;
    stripAudio_ = stripAudio;
    prefix_ = prefix;

    emit gogo();
}

TrimmerWorker::~TrimmerWorker()
{
}

void TrimmerWorker::trimAll()
{
    if( fcpxmlFile_.isEmpty() )
    {
        emit debug("ERROR: No FCPXML File specified!");
    }
    else if( outputFolder_.isEmpty() )
    {
        emit debug("ERROR: No output folder specified!");
    }
    else
    {
        QFile xmlFile(fcpxmlFile_);

        if ( !xmlFile.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            emit debug("ERROR: Couldn't open FCPXML file!");
        }
        else
        {
            QXmlStreamReader xmlStream(xmlFile.readAll());

            if( xmlStream.hasError() )
            {
                emit debug("ERROR: Couldn't parse FCPXML file!");
            }
            else
            {
                parse_fcpxml(xmlStream);
            }

            xmlFile.close();
        }
    }

    if( errorCount_ > 0 )
    {
        emit debug("******** There were errors **************");
    }
    else
    {
        emit debug("Finished");
    }

    emit allDone();
}

void TrimmerWorker::parse_fcpxml(QXmlStreamReader& xmlStream)
{
    if( xmlStream.readNextStartElement() )
    {
        if( xmlStream.name() == "fcpxml" )
        {
            if( xmlStream.attributes().value("version").toString() == "1.8")
            {
                while( xmlStream.readNextStartElement() )
                {
                    if( xmlStream.name() == "resources" )
                    {
                        debug("resources:");
                        parse_resources(xmlStream);
                    }
                    else if( xmlStream.name() == "library" )
                    {
                        debug("library:");
                        parse_library(xmlStream);
                    }
                    else
                    {
                        xmlStream.skipCurrentElement();
                    }
                }
            }
            else
            {
                emit debug("ERROR: Wrong FCPXML version!");
            }
        }
        else
        {
            emit debug("ERROR_XML: No fcpxml section!");
        }
    }
    else
    {
        emit debug("ERROR_XML: Major error!");
    }
}

void TrimmerWorker::parse_resources(QXmlStreamReader& xmlStream)
{
    while( xmlStream.readNextStartElement() )
    {
        if( xmlStream.name() == "asset" )
        {
            parse_asset(xmlStream);
        }
        else
        {
            xmlStream.skipCurrentElement();
        }
    }
}

void TrimmerWorker::parse_asset(QXmlStreamReader& xmlStream)
{
    if( xmlStream.attributes().value("hasVideo").toString() == "1" )
    {
        Asset asset;
        asset.id = xmlStream.attributes().value("id").toString();
        asset.name = xmlStream.attributes().value("name").toString();

        asset.src = resolveFilename(xmlStream.attributes().value("src").toString());
        debug("  asset " + asset.id + ": " + asset.name + " (" + asset.src + ")");

        assetMap.insert(asset.id, asset);
    }

    QString text = xmlStream.readElementText();
    Q_UNUSED(text);
}

void TrimmerWorker::parse_library(QXmlStreamReader& xmlStream)
{
    while( xmlStream.readNextStartElement() )
    {
        if( xmlStream.name() == "event" )
        {
            debug("  event: " + xmlStream.attributes().value("name").toString());
            parse_event(xmlStream);
        }
        else
        {
            xmlStream.skipCurrentElement();
        }
    }
}

void TrimmerWorker::parse_event(QXmlStreamReader& xmlStream)
{
    while( xmlStream.readNextStartElement() )
    {
        if( xmlStream.name() == "project" )
        {
            debug("    project: " + xmlStream.attributes().value("name").toString());
            parse_project(xmlStream);
        }
        else
        {
            xmlStream.skipCurrentElement();
        }
    }
}

void TrimmerWorker::parse_project(QXmlStreamReader& xmlStream)
{
    while( xmlStream.readNextStartElement() )
    {
        if( xmlStream.name() == "sequence" )
        {
            debug("      sequence:");
            parse_sequence(xmlStream);
        }
        else
        {
            xmlStream.skipCurrentElement();
        }
    }
}

void TrimmerWorker::parse_sequence(QXmlStreamReader& xmlStream)
{
    while( xmlStream.readNextStartElement() )
    {
        if( xmlStream.name() == "spine" )
        {
            debug("        spine:");
            parse_spine(xmlStream);
        }
        else
        {
            xmlStream.skipCurrentElement();
        }
    }
}

void TrimmerWorker::parse_spine(QXmlStreamReader& xmlStream)
{
    while( xmlStream.readNextStartElement() )
    {
        if( xmlStream.name() == "asset-clip" )
        {
            parse_asset_clip(xmlStream);

            QString text = xmlStream.readElementText();
            Q_UNUSED(text);
        }
        else
        {
            xmlStream.skipCurrentElement();
        }
    }
}

void TrimmerWorker::parse_asset_clip(QXmlStreamReader& xmlStream)
{
    QString ref = xmlStream.attributes().value("ref").toString();
    QString start = xmlStream.attributes().value("start").toString();
    QString duration = xmlStream.attributes().value("duration").toString();

    debug("          asset-clip: ref[" + ref +"] Start [" + start + "] " + "Duration[" + duration + "]");

    QMap<QString, Asset>::iterator it = assetMap.find(ref);

    if( it != assetMap.end() )
    {
        float start_t = extractTime(start);
        float duration_t = extractTime(duration);

        if( it->src.size() > 0 )
        {
            QFileInfo fileInfo(it->src);
            QString program("ffmpeg.exe");
            QString arguments("-hide_banner -i \"" + it->src + "\"");
            char temp[320];

            sprintf(temp, " -y -ss %5.3f -t %5.3f", start_t, duration_t);
            arguments += temp;
            arguments += " -vcodec copy";

            if( stripAudio_ )
            {
                arguments += " -an";
            }
            else
            {
                arguments += " -acodec copy";
            }

            QString prefix(prefix_);
            if( prefix.size() > 0 ) prefix += "_";

            sprintf(temp, " \"%s/%s%s__%06lld_%06lld.%s\"", outputFolder_.toStdString().c_str(), prefix.toStdString().c_str(), fileInfo.baseName().toStdString().c_str(), (long long int)(start_t*1000000.0f), (long long int)(duration_t*1000000.0f), fileInfo.suffix().toStdString().c_str());
            arguments += temp;

            emit debug( "            " + program + " " + arguments + "\n" );

            process_.setProgram( program + " " + arguments );
            process_.start();

            while( !process_.waitForFinished() )
            {
                ; // do nothing
            }

            if( process_.exitCode() != 0 )
            {
                emit debug("FAILED!");
                errorCount_++;
            }
        }
    }
    else
    {
        debug("            No video asset for this asset-clip - ignoring");
    }


    while( xmlStream.readNextStartElement() )
    {
        xmlStream.skipCurrentElement();
    }

    debug("");
}

bool TrimmerWorker::doesFileExist(const QString& fileName)
{
    bool doesExist = false;
    QFileInfo checker(fileName);

    if( checker.exists() && checker.isFile() )
    {
        doesExist = true;
    }

    return doesExist;
}

QString TrimmerWorker::resolveFilename(QString src)
{
    QUrl url(src);
    QStringList file_path_list = url.toString().split("/", QString::SkipEmptyParts);
    QString tryPath;
    QString foundPath;

    for(int i=file_path_list.size()-1; i>=0; i--)
    {
        if( tryPath.size() == 0 )
        {
            tryPath = file_path_list.at(i);
        }
        else
        {
            tryPath = file_path_list.at(i) + "/" + tryPath;
        }

        if( doesFileExist(tryPath) )
        {
            foundPath = tryPath;
        }
        else
        {
            ;   // keep iterating
        }
    }

    return foundPath;
}

float TrimmerWorker::extractTime(QString& str)
{
    double numerator = 0.0;
    float denominator = 1.0f;

    if( str.size() < 4 )
    {
        debug("ERROR: badly formatted time (bad size)");
        errorCount_++;
    }
    if( str.at(str.size()-1) != 's' )
    {
        debug("ERROR: badly formatted time (no 's')");
        errorCount_++;
    }
    else
    {
        str.remove(str.size()-1, 1);
        QStringList t_list = str.split("/", QString::SkipEmptyParts);

        if( t_list.size() != 2 )
        {
            debug("ERROR: badly formatted time (not enough pieces!)");
            errorCount_++;
        }
        else
        {
            QString n = t_list.at(0);
            numerator = n.toDouble();

            QString d = t_list.at(1);
            denominator = d.toFloat();
        }
    }

    return static_cast<float>(numerator/denominator);
}

void TrimmerWorker::on_readyReadStandardError()
{
    QByteArray ba = process_.readAllStandardError();
    emit cerr( QString(ba) );
    QThread::msleep(500);
}

void TrimmerWorker::on_readyReadStandardOutput()
{
    QByteArray ba = process_.readAllStandardOutput();
    emit cout( QString(ba) );
    QThread::msleep(500);
}

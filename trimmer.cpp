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
            QString program("ffmpeg");
            QString arguments("-i \"" + it->src + "\"");
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

            emit debug("            " + program + " " + arguments);

            if( system(QString(program + " " + arguments).toStdString().c_str()) != 0 )
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

//void TrimmerWorker::parse_video(QXmlStreamReader& xmlStream)
//{
//    while( xmlStream.readNextStartElement() )
//    {
//        if( xmlStream.name() == "track" )
//        {
//            parse_track(xmlStream);
//        }
//        else
//        {
//            xmlStream.skipCurrentElement();
//        }
//    }
//}

//void TrimmerWorker::parse_track(QXmlStreamReader& xmlStream)
//{
//    while( xmlStream.readNextStartElement() )
//    {
//        if( xmlStream.name() == "clipitem" )
//        {
//            parse_clipitem(xmlStream);
//        }
//        else
//        {
//            xmlStream.skipCurrentElement();
//        }
//    }
//}

//void TrimmerWorker::parse_clipitem(QXmlStreamReader& xmlStream)
//{
//     ClipItem clipItem;
//     clipItem.filey = nullptr;

//     while( xmlStream.readNextStartElement() )
//     {
//        if( xmlStream.name() == "name" )
//        {
//            clipItem.name = xmlStream.readElementText();
//        }
//        else if (xmlStream.name() == "rate" )
//        {
//            parse_clipitem_rate(xmlStream, clipItem);
//        }
//        else if( xmlStream.name() == "file" )
//        {
//            parse_clipitem_file(xmlStream, clipItem);
//        }
//        else if( xmlStream.name() == "subclipinfo" )
//        {
//            parse_clipitem_subclipinfo(xmlStream, clipItem);
//        }
//        else if( xmlStream.name() == "in" )
//        {
//            clipItem.file_in = xmlStream.readElementText();
//        }
//        else if( xmlStream.name() == "out" )
//        {
//            clipItem.file_out = xmlStream.readElementText();
//        }
//        else
//        {
//            xmlStream.skipCurrentElement();
//        }
//    }

//    trim(clipItem);
//}

//void TrimmerWorker::parse_clipitem_rate(QXmlStreamReader& xmlStream, ClipItem& clipItem)
//{
//   while( xmlStream.readNextStartElement() )
//   {
//      if( xmlStream.name() == "timebase" )
//      {
//          clipItem.timebase = xmlStream.readElementText();
//      }
//      else
//      {
//          xmlStream.skipCurrentElement();
//      }
//  }
//}

//void TrimmerWorker::parse_clipitem_file(QXmlStreamReader& xmlStream, ClipItem& clipItem)
//{
//    File filey;
//    filey.id = xmlStream.attributes().value("id").toString();

//    while( xmlStream.readNextStartElement() )
//    {
//       if( xmlStream.name() == "name" )
//       {
//           filey.name = xmlStream.readElementText();
//       }
//       else if( xmlStream.name() == "pathurl" )
//       {
//           filey.pathurl = xmlStream.readElementText();
//       }
//       else if( xmlStream.name() == "rate" )
//       {
//           parse_clipitem_file_rate(xmlStream, filey);
//       }
//       else
//       {
//           xmlStream.skipCurrentElement();
//       }
//   }

//    if( filey.id.size() > 0 )
//    {
//        if( (filey.name.size() == 0) || (filey.pathurl.size() == 0) || (filey.timebase.size() == 0) )
//        {
//            // this looks like a reference to a master clip - try and link it
//            QMap<QString, File>::iterator it = fileMap.find(filey.id);

//            if( it != fileMap.end() )
//            {
//                clipItem.filey = &(*it);
//            }
//            else
//            {
//                emit debug("*********** NO MASTER CLIP REFERENCE IN FILE MAP ***************");
//            }
//        }
//        else
//        {
//            // this looks like a master clip .................................
//            QMap<QString, File>::iterator it = fileMap.find(filey.id);

//            if( it != fileMap.end() )
//            {
//                // this master clip is already described, find and check
//                if( (it->name != filey.name) || (it->pathurl != filey.pathurl) || (it->timebase != filey.timebase))
//                {
//                    emit debug("*********** DUPLICATE IN FILE MAP ***************");
//                }

//                // link this clip item to existing good master clip
//                clipItem.filey = &(*it);
//            }
//            else
//            {
//                // master clip not in map, add and link it
//                fileMap[filey.id] = filey;
//                it = fileMap.find(filey.id);
//                clipItem.filey = &(*it);
//            }
//        }
//    }

//    resolveFilename(clipItem);
//}

//void TrimmerWorker::parse_clipitem_file_rate(QXmlStreamReader& xmlStream, File& filey)
//{
//   while( xmlStream.readNextStartElement() )
//   {
//      if( xmlStream.name() == "timebase" )
//      {
//          filey.timebase = xmlStream.readElementText();
//      }
//      else
//      {
//          xmlStream.skipCurrentElement();
//      }
//  }
//}

//void TrimmerWorker::parse_clipitem_subclipinfo(QXmlStreamReader& xmlStream, ClipItem& clipItem)
//{
//    while( xmlStream.readNextStartElement() )
//    {
//       if( xmlStream.name() == "startoffset" )
//       {
//           clipItem.file_subclipinfo_startoffset = xmlStream.readElementText();
//       }
//       else if( xmlStream.name() == "endoffset" )
//       {
//           clipItem.file_subclipinfo_endoffset = xmlStream.readElementText();
//       }
//       else
//       {
//           xmlStream.skipCurrentElement();
//       }
//   }
//}

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

//void TrimmerWorker::trim(ClipItem& clipItem)
//{
//    emit debug("");
//    emit debug("    Clip Item: " + clipItem.name);

//    if( nullptr == clipItem.filey )
//    {
//        emit debug("      ERROR: Filey nullptr!");
//        errorCount_++;
//    }
//    else
//    {
//        int timebase = clipItem.timebase.toInt(); //clipItem.filey->timebase.toInt();

//        emit debug("      File Id: " + clipItem.filey->id);
//        emit debug("      File Name: " + clipItem.filey->name);
//        emit debug("      File URL: " + clipItem.filey->pathurl);
//        emit debug("      File Timebase:  " + clipItem.filey->timebase + "fps (Clip Timebase: " + clipItem.timebase + "fps)");
//        emit debug("      File In:  " + clipItem.file_in + " frames");
//        emit debug("      File Out:  " + clipItem.file_out + " frames");
//        emit debug("      File Subclip Start:  " + clipItem.file_subclipinfo_startoffset + " frames");
//        emit debug("      File Subclip End:  " + clipItem.file_subclipinfo_endoffset + " frames");

//        if( clipItem.filey->timebase != clipItem.timebase )
//        {
//            emit debug("      ERROR: Frame rates don't match!");
//            errorCount_++;
//        }

//        qint32 startFrame = clipItem.file_subclipinfo_startoffset.toInt() + clipItem.file_in.toInt();
//        qint32 endFrame = clipItem.file_subclipinfo_startoffset.toInt() + clipItem.file_out.toInt();
//        float startSecs = static_cast<float>(startFrame) / static_cast<float>(timebase);
//        float endSecs = static_cast<float>(endFrame) / static_cast<float>(timebase);
//        float durationSecs = endSecs - startSecs;

//        if( clipItem.filey->pathurl.size() > 0 )
//        {
//            QFileInfo fileInfo(clipItem.filey->pathurl);
//            QString program("ffmpeg");
//            QString arguments("-i \"" + clipItem.filey->pathurl + "\"");
//            char temp[320];

//            sprintf(temp, " -y -ss %5.3f -t %5.3f", startSecs, durationSecs);
//            arguments += temp;
//            arguments += " -vcodec copy";

//            if( stripAudio_ )
//            {
//                arguments += " -an";
//            }
//            else
//            {
//                arguments += " -acodec copy";
//            }

//            QString prefix(prefix_);
//            if( prefix.size() > 0 ) prefix += "_";

//            sprintf(temp, " \"%s/%s%s__%06d_%06d.%s\"", outputFolder_.toStdString().c_str(), prefix.toStdString().c_str(), fileInfo.baseName().toStdString().c_str(), startFrame, endFrame, fileInfo.suffix().toStdString().c_str());
//            arguments += temp;

//            emit debug("");
//            emit debug("      Calling: " + program + " " + arguments);

//            if( system(QString(program + " " + arguments).toStdString().c_str()) != 0 )
//            {
//                emit debug("FAILED!");
//                errorCount_++;
//            }
//        }
//    }
//}

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

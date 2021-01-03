#include "ffprobewrapper.h"

FFprobeWrapper::FFprobeWrapper(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    connect(m_process, &QProcess::started, this, &FFprobeWrapper::started);
    connect(m_process, &QProcess::errorOccurred, this, &FFprobeWrapper::errorOccured);
    connect(m_process, &QProcess::readyReadStandardError, this, &FFprobeWrapper::readyReadStandardError);
    connect(m_process, &QProcess::readyRead, this, &FFprobeWrapper::readyRead);
    connect(m_process, &QProcess::stateChanged, this, &FFprobeWrapper::stateChanged);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FFprobeWrapper::finished);
}

void FFprobeWrapper::getFileData(const QString &path)
{
    m_process->start("ffprobe",
                     QStringList()
                     << "-show_entries"
                     << "stream"
                     << "-show_format"
                     << "-sexagesimal"
                     << "-of" << "json"
                     << path
                     << "-v" << "0");
    m_resultJson.clear();
}

void FFprobeWrapper::started()
{
    qDebug() << "process started";
}

void FFprobeWrapper::errorOccured(QProcess::ProcessError error)
{
    qDebug() << "error occured:" << error;
}

void FFprobeWrapper::readyReadStandardError()
{
    qDebug() << "stderr: " << m_process->readAllStandardError();
}

void FFprobeWrapper::readyRead()
{
    m_resultJson += m_process->readAll();
}

void FFprobeWrapper::stateChanged(QProcess::ProcessState newState)
{
    qDebug() << "state changed: " << newState;
}

void FFprobeWrapper::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    QJsonDocument jsonDocument = QJsonDocument::fromJson(m_resultJson);
    QJsonArray jsonArray = jsonDocument["streams"].toArray();
    QVariantList bufferList = jsonArray.toVariantList();
    QStringList duration = jsonDocument["format"].toObject()["duration"].toString().split('.')[0].split(':');
    foreach(const QVariant &data, bufferList) {
        QMap<QString, QVariant> temp = data.toMap();
        unsigned short index = temp.value("index").toUInt();
        QString codecName = temp.value("codec_name").toString();
        QString codecLongName = temp.value("codec_long_name").toString();
        QString codecType = temp.value("codec_type").toString();
        QString width = temp.value("width").toString();
        QString height = temp.value("height").toString();
        QString channels = temp.value("channels").toString();
        QString sampleRate = temp.value("sample_rate").toString();
        int totalFrames = 0;
        QString avgFrameRate = temp.value("avg_frame_rate").toString();
        if (avgFrameRate != "0/0") {
            unsigned short hours = duration[0].toUInt();
            unsigned short minutes = duration[1].toUInt();
            unsigned short seconds = duration[2].toUInt();
            QStringList frameRateComponents = avgFrameRate.split('/');
            int frameRate = frameRateComponents[0].toFloat() / frameRateComponents[1].toFloat();
            totalFrames = (hours * 3600 + minutes * 60 + seconds) * frameRate;
        }
        QString features;
        if (codecType == "video") features = width + "x" + height + " пикселей";
        if (codecType == "audio") features = sampleRate + " Гц, " + channels + " канала(ов)";
        QString language;
        QString title;
        QMap<QString, QVariant> tags = temp.value("tags").toMap();
        foreach(const QString &key, tags.keys()) {
            QString value = tags.value(key).toString();
            if (key == "language") language = value;
            if (key == "title") title = value;
        }
        QMap<QString, QString> resultMap;
        QList<QMap<QString, QString>> resultList;
        resultMap.insert("codecName", codecName);
        resultMap.insert("codecLongName", codecLongName);
        resultMap.insert("codecType", codecType);
        resultMap.insert("language", language);
        resultMap.insert("title", title);
        resultMap.insert("features", features);
        resultMap.insert("totalFrames", QString::number(totalFrames));
        resultList.append(resultMap);
        emit dataCollected(index, resultList);
    }
}

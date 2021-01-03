#include "ffmpegwrapper.h"

FFmpegWrapper::FFmpegWrapper(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    connect(m_process, &QProcess::started, this, &FFmpegWrapper::started);
    connect(m_process, &QProcess::errorOccurred, this, &FFmpegWrapper::errorOccured);
    connect(m_process, &QProcess::readyReadStandardError, this, &FFmpegWrapper::readyReadStandardError);
    connect(m_process, &QProcess::readyRead, this, &FFmpegWrapper::readyRead);
    connect(m_process, &QProcess::stateChanged, this, &FFmpegWrapper::stateChanged);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FFmpegWrapper::finished);
}

void FFmpegWrapper::start(const QString &sourcePath, const QString &destinationPath, const QStringList &streamList)
{
    m_process->start("ffmpeg",
                     QStringList()
                     << "-i" << sourcePath
                     << "-c:v" << "copy"
                     << "-ac" << "2"
                     << "-c:a" << "aac"
                     << "-ar" << "48000"
                     << "-b:a" << "192k"
                     << streamList
                     << destinationPath << "-y"
                     << "-stats" << "-hide_banner" << "-loglevel" << "panic"
                     << "-progress" << "pipe:1"
                     );
}

void FFmpegWrapper::started()
{
    emit convertingStarted();
}

void FFmpegWrapper::errorOccured(QProcess::ProcessError error)
{
    qDebug() << "ffmpeg error occured:" << error;
}

void FFmpegWrapper::readyReadStandardError()
{
//    qDebug().noquote() << "stderr: " << m_process->readAllStandardError().trimmed();
}

void FFmpegWrapper::readyRead()
{
    QString output =  m_process->readAll().trimmed();
    emit currentProgress(output.split('\n')[0].split('=')[1].toUInt());
}

void FFmpegWrapper::stateChanged(QProcess::ProcessState newState)
{
    qDebug() << "ffmpeg state changed: " << newState;
}

void FFmpegWrapper::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    emit convertingFinished();
}

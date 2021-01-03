#include "normalizewrapper.h"

NormalizeWrapper::NormalizeWrapper(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    connect(m_process, &QProcess::started, this, &NormalizeWrapper::started);
    connect(m_process, &QProcess::errorOccurred, this, &NormalizeWrapper::errorOccured);
    connect(m_process, &QProcess::readyReadStandardError, this, &NormalizeWrapper::readyReadStandardError);
    connect(m_process, &QProcess::readyRead, this, &NormalizeWrapper::readyRead);
    connect(m_process, &QProcess::stateChanged, this, &NormalizeWrapper::stateChanged);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &NormalizeWrapper::finished);
}

void NormalizeWrapper::start(const QString &sourceFile, const QString &destinationFile)
{
    m_process->start("ffmpeg-normalize",
                     QStringList()
                     << sourceFile
                     << "-c:a" << "aac"
                     << "-ar" << "48000"
                     << "-b:a" << "192k"
                     << "-o" << destinationFile
                     << "-pr" << "-f"
                     );
}

void NormalizeWrapper::started()
{
    qDebug() << "normalizer process started";
}

void NormalizeWrapper::errorOccured(QProcess::ProcessError error)
{
    qDebug() << "normalizer error occured:" << error;
}

void NormalizeWrapper::readyReadStandardError()
{
    QString output = m_process->readAllStandardError().trimmed();
    if (!output.isEmpty()) {
        if (output.contains("Stream")) {
            QStringList outputParts = output.split("Stream")[1].trimmed().split(':');
            emit updateStatus("Нормализация, первый проход. Обработка дорожки: " + outputParts[0]);
            emit updateProgress(outputParts[1].split('|')[0].trimmed().remove('%').toUInt());
        }
        if (output.contains("Second Pass")) {
            emit updateStatus("Нормализация, второй проход");
            emit updateProgress(output.split(':')[1].split('|')[0].trimmed().remove('%').toUInt());
        }
    }
}

void NormalizeWrapper::readyRead()
{
    qDebug() << "normalizer stdout: " << m_process->readAll().trimmed();
}

void NormalizeWrapper::stateChanged(QProcess::ProcessState newState)
{
    qDebug() << "normalizer state changed: " << newState;
}

void NormalizeWrapper::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    emit updateStatus("Нормализация завершена");
    emit normalizeFinished();
}

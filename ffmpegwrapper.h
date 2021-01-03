#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QDebug>

class FFmpegWrapper : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegWrapper(QObject *parent = nullptr);

    void start(const QString &sourcePath, const QString &destinationPath, const QStringList &streamList);

signals:
    void currentProgress(const unsigned int &);
    void convertingStarted();
    void convertingFinished();

private:
    QProcess *m_process;

private slots:
    void started();
    void errorOccured(QProcess::ProcessError error);
    void readyReadStandardError();
    void readyRead();
    void stateChanged(QProcess::ProcessState newState);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // FFMPEGWRAPPER_H

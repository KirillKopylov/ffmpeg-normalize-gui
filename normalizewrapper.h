#ifndef NORMALIZEWRAPPER_H
#define NORMALIZEWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QDebug>

class NormalizeWrapper : public QObject
{
    Q_OBJECT
public:
    explicit NormalizeWrapper(QObject *parent = nullptr);

    void start(const QString &sourceFile, const QString &destinationFile);

signals:
    void updateStatus(const QString &);
    void updateProgress(const unsigned int &);
    void normalizeFinished();

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

#endif // NORMALIZEWRAPPER_H

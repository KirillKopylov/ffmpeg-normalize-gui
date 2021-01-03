#ifndef FFPROBEWRAPPER_H
#define FFPROBEWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class FFprobeWrapper : public QObject
{
    Q_OBJECT
public:
    explicit FFprobeWrapper(QObject *parent = nullptr);
    void getFileData(const QString &path);

signals:
    void dataCollected(const unsigned short &, const QList<QMap<QString, QString>> &);

private:
    QProcess *m_process;
    QByteArray m_resultJson;

private slots:
    void started();
    void errorOccured(QProcess::ProcessError error);
    void readyReadStandardError();
    void readyRead();
    void stateChanged(QProcess::ProcessState newState);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // FFPROBEWRAPPER_H

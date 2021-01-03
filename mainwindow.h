#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QMessageBox>

#include "ffprobewrapper.h"
#include "ffmpegwrapper.h"
#include "normalizewrapper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    FFprobeWrapper *m_FFprobeWrapper;
    FFmpegWrapper *m_FFmpegWrapper;
    NormalizeWrapper *m_NormalizeWrapper;
    enum TableColumns {
        Index = 0,
        Type,
        Title,
        Language,
        Features,
        CodecName,
        CodecLongName
    };
    QString m_sourceFile;
    QString m_destinationFile;


private slots:
    void selectSourceFile();
    void selectDestinationFile();
    void setStream(const unsigned short &index, const QList<QMap<QString, QString>> &data);
    void start();
    void updateProgressBar(const unsigned int &currentProgress);
    void stereoConvertingFinished();
    void updateStatusBar(const QString &message);
    void normalizeFinished();
};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_FFprobeWrapper = new FFprobeWrapper(this);
    m_FFmpegWrapper = new FFmpegWrapper(this);
    m_NormalizeWrapper = new NormalizeWrapper(this);
    connect(ui->buttonSourcePath, &QPushButton::clicked, this, &MainWindow::selectSourceFile);
    connect(ui->buttonDestinationPath, &QPushButton::clicked, this, &MainWindow::selectDestinationFile);
    connect(m_FFprobeWrapper, &FFprobeWrapper::dataCollected, this, &MainWindow::setStream);
    connect(ui->buttonProceed, &QPushButton::clicked, this, &MainWindow::start);
    connect(ui->checkBoxRewriteSource, &QCheckBox::clicked, this, [this] (bool checked) {
        ui->lineEditDestinationPath->setDisabled(checked);
        ui->buttonDestinationPath->setDisabled(checked);
    });
    connect(m_FFmpegWrapper, &FFmpegWrapper::currentProgress, this, &MainWindow::updateProgressBar);
    connect(m_FFmpegWrapper, &FFmpegWrapper::convertingFinished, this, &MainWindow::stereoConvertingFinished);
    connect(m_NormalizeWrapper, &NormalizeWrapper::updateProgress, this, &MainWindow::updateProgressBar);
    connect(m_NormalizeWrapper, &NormalizeWrapper::updateStatus, this, &MainWindow::updateStatusBar);
    connect(m_NormalizeWrapper, &NormalizeWrapper::normalizeFinished, this, &MainWindow::normalizeFinished);
    connect(m_FFmpegWrapper, &FFmpegWrapper::convertingStarted, this, [=] {
        ui->buttonDestinationPath->setDisabled(true);
        ui->buttonSourcePath->setDisabled(true);
        ui->buttonProceed->setDisabled(true);
    });
    QStringList header;
    header << "Индекс" << "Тип" << "Название" << "Язык" << "Свойства"
           << "Название кодека" << "Название кодека полностью";
    ui->tableWidget->setColumnCount(header.count());
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->progressBar->hide();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectSourceFile()
{
    m_sourceFile = QFileDialog::getOpenFileName(this);
    if (m_sourceFile.isEmpty()) return;
    QStringList splittedFileName = m_sourceFile.split('.');
    QString fileExtension = splittedFileName.last();
    m_destinationFile = m_sourceFile;
    m_destinationFile.remove('.' + fileExtension) += "_normalized." + fileExtension;
    ui->lineEditSourcePath->setText(m_sourceFile);
    ui->lineEditDestinationPath->setText(m_destinationFile);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    m_FFprobeWrapper->getFileData(m_sourceFile);
}

void MainWindow::selectDestinationFile()
{
    m_destinationFile = QFileDialog::getSaveFileName(this);
    if (m_destinationFile.isEmpty()) return;
    ui->lineEditDestinationPath->setText(m_destinationFile);
}

void MainWindow::setStream(const unsigned short &index, const QList<QMap<QString, QString>> &data)
{
    QMap<QString, QString> items = data.at(0);
    unsigned int totalFrames = items.value("totalFrames").toUInt();
    if (totalFrames) ui->progressBar->setMaximum(totalFrames);
    ui->tableWidget->insertRow(index);
    QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(index));
    QTableWidgetItem *codecTypeItem = new QTableWidgetItem(items.value("codecType"));
    QTableWidgetItem *titleItem = new QTableWidgetItem(items.value("title"));
    QTableWidgetItem *languageItem = new QTableWidgetItem(items.value("language"));
    QTableWidgetItem *codecNameItem = new QTableWidgetItem(items.value("codecName"));
    QTableWidgetItem *codecLongNameItem = new QTableWidgetItem(items.value("codecLongName"));
    QTableWidgetItem *featuresItem = new QTableWidgetItem(items.value("features"));
    indexItem->setFlags(indexItem->flags() ^ Qt::ItemIsEditable);
    codecTypeItem->setFlags(codecTypeItem->flags() ^ Qt::ItemIsEditable);
    titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);
    languageItem->setFlags(languageItem->flags() ^ Qt::ItemIsEditable);
    codecNameItem->setFlags(codecNameItem->flags() ^ Qt::ItemIsEditable);
    codecLongNameItem->setFlags(codecLongNameItem->flags() ^ Qt::ItemIsEditable);
    featuresItem->setFlags(featuresItem->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(index, TableColumns::Index, indexItem);
    ui->tableWidget->setItem(index, TableColumns::Type, codecTypeItem);
    ui->tableWidget->setItem(index, TableColumns::Title, titleItem);
    ui->tableWidget->setItem(index, TableColumns::Language, languageItem);
    ui->tableWidget->setItem(index, TableColumns::Features, featuresItem);
    ui->tableWidget->setItem(index, TableColumns::CodecName, codecNameItem);
    ui->tableWidget->setItem(index, TableColumns::CodecLongName, codecLongNameItem);
    ui->tableWidget->resizeColumnsToContents();
}

void MainWindow::start()
{
    QModelIndexList selectedRows = ui->tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Ни один поток не выбран", QMessageBox::Ok);
        return;
    }
    if (ui->checkBoxRewriteSource->isChecked()) {
        m_destinationFile = m_sourceFile;
    }
    if (m_destinationFile.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран результирующий файл", QMessageBox::Ok);
        return;
    }
    QStringList selectedStreams;
    for(const QModelIndex &index : qAsConst(selectedRows)) {
        selectedStreams << "-map" << "0:" + QString::number(index.row());
    }
    m_FFmpegWrapper->start(m_sourceFile, m_destinationFile, selectedStreams);
    updateStatusBar("Преобразование звуковых дорожек в стерео");
    ui->progressBar->show();
}

void MainWindow::updateProgressBar(const unsigned int &currentProgress)
{
    ui->progressBar->setValue(currentProgress);
}

void MainWindow::stereoConvertingFinished()
{
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(100);
    m_NormalizeWrapper->start(m_destinationFile, m_destinationFile);
}

void MainWindow::updateStatusBar(const QString &message)
{
    ui->statusbar->showMessage(message);
}

void MainWindow::normalizeFinished()
{
    ui->progressBar->hide();
    ui->progressBar->setValue(0);
    ui->buttonDestinationPath->setDisabled(false);
    ui->buttonSourcePath->setDisabled(false);
    ui->buttonProceed->setDisabled(false);
    updateStatusBar("Нормализация завершена");
}

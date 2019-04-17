#include "verifierwindow.h"
#include "ui_verifierwindow.h"
#include "hashworker.h"
#include "aboutdialog.h"
#include "operationdialog.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>

VerifierWindow::VerifierWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VerifierWindow)
{
    ui->setupUi(this);

    ui->actionOperation->setEnabled(false);

    ui->treeWidget->setColumnWidth(0, 200);
    ui->treeWidget->setColumnWidth(1, 100);
    ui->treeWidget->setColumnWidth(2, 120);

    icon[0] = QIcon(":/images/default.png");
    icon[1] = QIcon(":/images/ok.png");
    icon[2] = QIcon(":/images/bad.png");
    icon[3] = QIcon(":/images/missing.png");

    //qDebug() << "checksum file:" << qApp->arguments().at(1);
    Settings::init();
    if(!ReadFile()) return;

    // StatusBar
    statusLabel = new QLabel("");
    statusLabel->setIndent(8);
    ui->statusbar->addWidget(statusLabel);
    ui->statusbar->setStyleSheet("QStatusBar::item{ border: 0 };");
    //ui->statusbar->showMessage("Checking...");

}

VerifierWindow::~VerifierWindow()
{
    workerThread.requestInterruption();
    workerThread.quit();
    workerThread.wait();
    delete ui;
}

bool VerifierWindow::ReadFile()
{
    QString checksumFile = qApp->arguments().at(1);
    QFile file(checksumFile);

    if(!file.exists())
    {
        QMessageBox::critical(this, "Checksum Verifier", "Checksum file not found.");
        return false;
    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Checksum Verifier", "Access denied.");
        return false;
    }

    QString fileExt = QFileInfo(file).suffix();

    QString pattern;
    int hashIdx = 1, fileIdx = 2;
    if(fileExt == "md5")
    {
        hashType = 0;
        ui->treeWidget->setHeaderLabels( { "Name", "Status", "Checksum (MD5)" } );
        pattern = QString(R"(^\s*([0-z]{32})\s[\s|\*]*(.*[^\s])\s*$)");
    }
    else if(fileExt == "sha1")
    {
        hashType = 1;
        ui->treeWidget->setHeaderLabels( { "Name", "Status", "Checksum (SHA-1)" } );
        pattern = QString(R"(^\s*([0-z]{40})\s[\s|\*]*(.*[^\s])\s*$)");
    }
    else if(fileExt == "sfv")
    {
        hashType = 2;
        ui->treeWidget->setHeaderLabels( { "Name", "Status", "Checksum (CRC32)" } );
        pattern = QString(R"(^\s*(.*[^\s])\s[\s|\*]*([0-z]{8})\s*$)");
        hashIdx = 2;
        fileIdx = 1;
    }
    else {
        QMessageBox::critical(this, "Checksum Verifier", "Unknown hash type.");
        return false;
    }

    //QTextStream stream(&file);

    QRegularExpression re(pattern);
    while(!file.atEnd())
    {
        QString line = file.readLine();
        if(line.contains(';'))
            line = line.split(';').at(0);
        QRegularExpressionMatch match = re.match(line);
        //qDebug() << match.capturedTexts();

        if(match.hasMatch())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem( {match.captured(fileIdx), nullptr, match.captured(hashIdx)} );
            item->setIcon(0, icon[0]);
            ui->treeWidget->addTopLevelItem(item);
            filesCount++;
        }
    }
    file.close();

    ui->labelFiles->setText(QString("Files: 0/%0").arg(filesCount));
    rootPath = QFileInfo(file).path().append('/');
    return true;
}

void VerifierWindow::Verify()
{
    auto *worker = new HashWorker;
    worker->moveToThread(&workerThread);
    worker->tree = ui->treeWidget;
    worker->hashType = hashType;
    worker->verifyMode = true;
    worker->rootPath = &rootPath;

    connect(&workerThread, &QThread::started, worker, &HashWorker::doWork);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &HashWorker::finished, this, &VerifierWindow::finished);
    connect(worker, &HashWorker::progress, this, &VerifierWindow::showProgress);
    connect(worker, &HashWorker::hashReady, this, &VerifierWindow::showResult);
    connect(worker, &HashWorker::error, this, &VerifierWindow::showError);

    workerThread.start();
}

void VerifierWindow::finished()
{
    qDebug() << "Event::Finished";
    workerThread.quit();

    ui->checkBoxOK->setEnabled(true);
    ui->checkBoxBad->setEnabled(true);
    ui->checkBoxMissing->setEnabled(true);
    ui->actionOperation->setEnabled(true);
    ui->statusbar->showMessage("Finished.");
}

void VerifierWindow::showResult(int idx, const QByteArray& hash)
{
    //qDebug() << idx << hash;
    if(ui->treeWidget->topLevelItem(idx)->text(2).compare(hash, Qt::CaseInsensitive) == 0)
    {
        ui->treeWidget->topLevelItem(idx)->setText(1, "OK");
        ui->treeWidget->topLevelItem(idx)->setIcon(0, icon[1]);
        okCount++;
        ui->checkBoxOK->setText(QString("OK: %0").arg(okCount));
        okFiles.push_back(ui->treeWidget->topLevelItem(idx));
    }
    else
    {
        ui->treeWidget->topLevelItem(idx)->setText(1, "Hash mismatch");
        ui->treeWidget->topLevelItem(idx)->setIcon(0, icon[2]);
        badCount++;
        ui->checkBoxBad->setText(QString("Bad: %0").arg(badCount));
        badFiles.push_back(ui->treeWidget->topLevelItem(idx));
    }
    ui->labelFiles->setText(QString("Files: %0/%1").arg(checkedCount).arg(filesCount));
    checkedCount++;
}

void VerifierWindow::showProgress(int val, int idx)
{
    ui->treeWidget->topLevelItem(idx)->setText(1, QString("%0%").arg(val));
}

void VerifierWindow::showError(const QString& err, int idx)
{
    ui->treeWidget->topLevelItem(idx)->setText(1, err);
    missingCount++;
    ui->checkBoxMissing->setText(QString("Missing: %0").arg(missingCount));
    ui->treeWidget->topLevelItem(idx)->setIcon(0, icon[3]);
    ui->labelFiles->setText(QString("Files: %0/%1").arg(checkedCount).arg(filesCount));
    checkedCount++;
    missingFiles.push_back(ui->treeWidget->topLevelItem(idx));
}



void VerifierWindow::on_checkBoxOK_stateChanged(int state)
{
    bool checked = state == 2;
    foreach (QTreeWidgetItem *item, okFiles) {
        item->setHidden(!checked);
    }
}

void VerifierWindow::on_checkBoxBad_stateChanged(int state)
{
    bool checked = state == 2;
    foreach (QTreeWidgetItem *item, badFiles) {
        item->setHidden(!checked);
    }
}

void VerifierWindow::on_checkBoxMissing_stateChanged(int state)
{
    bool checked = state == 2;
    foreach (QTreeWidgetItem *item, missingFiles) {
        item->setHidden(!checked);
    }
}

void VerifierWindow::on_actionExit_triggered()
{
    this->close();
}

void VerifierWindow::on_actionAbout_triggered()
{
    AboutDialog ad(this);
    ad.exec();
}

void VerifierWindow::on_actionOperation_triggered()
{
    OperationDialog opd(this);
    opd.okFiles = &okFiles;
    opd.badFiles = &badFiles;
    opd.selectedFiles = ui->treeWidget->selectedItems();
    opd.exec();

    if(opd.result() == QDialog::Accepted)
    {
        qDebug() << QString("op: %1, target: %2, dest: %3").arg(opd.op).arg(opd.target).arg(opd.destPath);
    }
}


void VerifierWindow::showEvent(QShowEvent *event)
{   
    QWidget::showEvent(event);
    if(!firstTime)
        return;
    firstTime = false;
    Verify();
}

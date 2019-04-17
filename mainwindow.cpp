#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customqtreewidgetitem.h"
#include "hashworker.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include <QComboBox>
#include <QToolButton>
#include <QDebug>
#include <QFileDialog>
#include <QDirIterator>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QCloseEvent>
#include <QMimeData>


static QLocale enLocale(QLocale::English);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    startStopIcon[0] = QIcon(":/images/start.png");
    startStopIcon[1] = QIcon(":/images/stop.png");

    // Save Button
    saveButton = new QToolButton(this);
    saveButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    saveButton->setPopupMode(QToolButton::MenuButtonPopup);
    saveButton->setIcon(QIcon(":/images/save.png"));
    saveButton->setText("Save");
    connect(saveButton, &QAbstractButton::clicked, this, &MainWindow::saveButton_clicked);
    ui->mainToolBar->insertWidget(ui->mainToolBar->actions().at(5), saveButton);

    auto *saveMenu = new QMenu;
    QAction *saveAsAction = new QAction("Save as...");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsAction_triggered);
    saveMenu->addAction(saveAsAction);
    saveButton->setMenu(saveMenu);

    // Calculate Button
    calcButton = new QToolButton(this);
    calcButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    calcButton->setPopupMode(QToolButton::MenuButtonPopup);
    calcButton->setIcon(startStopIcon[0]);
    calcButton->setText("Calculate");
    connect(calcButton, &QAbstractButton::clicked, this, &MainWindow::calcButton_clicked);
    ui->mainToolBar->addWidget(calcButton);

    auto *calcMenu = new QMenu;
    QAction *restartAction = new QAction("Restart");
    connect(restartAction, &QAction::triggered, this, &MainWindow::restartAction_triggered);
    calcMenu->addAction(restartAction);
    calcButton->setMenu(calcMenu);

    // StatusBar
    statusLabel = new QLabel("0 File(s)");
    statusLabel->setIndent(8);
    ui->statusBar->addWidget(statusLabel);
    ui->statusBar->setStyleSheet("QStatusBar::item{ border: 0 };");

    ui->treeWidget->setColumnWidth(0, 220);
    ui->treeWidget->setColumnWidth(1, 220);
    ui->treeWidget->setColumnWidth(2, 80);
    ui->treeWidget->setColumnWidth(3, 120);
    ui->treeWidget->hideColumn(0);

    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortItems(1, Qt::AscendingOrder);

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showMenu);


    Settings::init();
    lastPath = Settings::lastPath();

}

MainWindow::~MainWindow()
{  
    workerThread.requestInterruption();
    workerThread.quit();
    workerThread.wait();
    delete ui;
}

// --- ADD FILES
void MainWindow::on_actionAdd_Files_triggered()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select File(s)", lastPath, nullptr, nullptr, QFileDialog::DontResolveSymlinks);
    if(files.count() > 0)
    {
        lastPath = QFileInfo(files.at(0)).path();
        ui->treeWidget->setUpdatesEnabled(false);
        foreach (QString fileName, files) {
            if(filesList.contains(fileName))
                continue;
            filesList.push_back(fileName);
            //qint64 fileSize = QFileInfo(fileName).size();
            QFile file(fileName);
            if(QFileInfo(file).isSymLink())
                file.open(QIODevice::ReadOnly);
            qint64 fileSize = file.size();
            totalSize += fileSize;
            ui->treeWidget->addTopLevelItem(new CustomQTreeWidgetItem(QStringList( {fileName, QFileInfo(fileName).fileName(), enLocale.formattedDataSize(fileSize), "-"} ), fileSize));
        }
        ui->treeWidget->setUpdatesEnabled(true);

        statusLabel->setText(QString::number(filesCount += files.count()) + " File(s) | " + enLocale.formattedDataSize(totalSize));
    }
}

// --- ADD FOLDER
void MainWindow::on_actionAdd_Folder_triggered()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select Folder", lastPath);

    if(!path.isEmpty())
    {
        lastPath = path;
        QDirIterator iterator(path, QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);
        int count = 0;

        ui->treeWidget->setUpdatesEnabled(false);
        while(iterator.hasNext()){
            QString fileName = iterator.next();
            if(filesList.contains(fileName))
                continue;
            filesList.push_back(fileName);
            //qint64 fileSize = iterator.fileInfo().size();
            QFile file(fileName);
            if(QFileInfo(file).isSymLink())
                file.open(QIODevice::ReadOnly);
            qint64 fileSize = file.size();
            totalSize += fileSize;
            QTreeWidgetItem *item = new CustomQTreeWidgetItem(QStringList( {fileName, iterator.fileInfo().fileName(), enLocale.formattedDataSize(fileSize), "-"} ), fileSize);
            ui->treeWidget->addTopLevelItem(item);
            count++;
        }
        ui->treeWidget->setUpdatesEnabled(true);

        statusLabel->setText(QString::number(filesCount += count) + " File(s) | " + enLocale.formattedDataSize(totalSize));
    }

}

// --- Toggle Full Path
void MainWindow::on_chkFullPath_stateChanged(int state)
{
    if(state == 2)
    {
        ui->treeWidget->setColumnWidth(0, ui->treeWidget->columnWidth(1));
        ui->treeWidget->hideColumn(1);
        ui->treeWidget->showColumn(0);
    }
    else
    {
        ui->treeWidget->setColumnWidth(1, ui->treeWidget->columnWidth(0));
        ui->treeWidget->hideColumn(0);
        ui->treeWidget->showColumn(1);
    }
}

void MainWindow::showHash(int r, const QByteArray& hash)
{
    ui->treeWidget->topLevelItem(r)->setText(3, hash);
}

void MainWindow::showError(const QString& err, int i)
{
    ui->treeWidget->topLevelItem(i)->setText(3, err);
}

void MainWindow::updateSize(qint64 diff, int idx)
{
    auto *item = static_cast<CustomQTreeWidgetItem*>(ui->treeWidget->topLevelItem(idx));
    item->setText(2, enLocale.formattedDataSize(item->size));
    totalSize += diff;
    statusLabel->setText(QString::number(filesCount) + " File(s) | " + enLocale.formattedDataSize(totalSize));
}

void MainWindow::finished()
{
    qDebug() << "SLOT::Finished";
    workerThread.quit();

    ToggleToolbar();
    ui->progressBar->reset();
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    if(!interrupt) {
        ui->statusBar->showMessage("█ Finished", 900);
    } else {
        ui->statusBar->showMessage("█ Stopped", 900);
        interrupt = false;
    }

}

void MainWindow::handleNext(int i)
{
    ui->treeWidget->scrollToItem(ui->treeWidget->topLevelItem(i));
}

void MainWindow::ToggleToolbar()
{
    calcButton->setText(started ? "Calculate" : "Stop");
    calcButton->setIcon(started ? startStopIcon[0] : startStopIcon[1]);
    calcButton->menu()->actions().at(0)->setEnabled(started);
    ui->actionAdd_Files->setEnabled(started);
    ui->actionAdd_Folder->setEnabled(started);
    ui->actionRemove->setEnabled(started);
    ui->actionClear->setEnabled(started);
    saveButton->setEnabled(started);
    ui->actionSettings->setEnabled(started);
    ui->treeWidget->setSortingEnabled(started);
    started = !started;
}

QString MainWindow::GetCommonRoot()
{
    QStringList commonRootList = ui->treeWidget->topLevelItem(0)->text(0).split('/');
    commonRootList.removeLast();
    int cParts = commonRootList.length();

    for (int i = 1; i < ui->treeWidget->topLevelItemCount() && cParts > 0; i++)
    {
        QStringList nextPath = ui->treeWidget->topLevelItem(i)->text(0).split('/');
        nextPath.removeLast();

        for (int j = 0; j < cParts; j++)
        {
            if(nextPath.length() == j || commonRootList.at(j) != nextPath.at(j))
            {
                cParts = j;
                while(commonRootList.length() > cParts)
                    commonRootList.removeLast();
            }
        }
    }

    return commonRootList.join('/');
}

QString MainWindow::GetFileExt()
{
    int hashLength = ui->treeWidget->topLevelItem(0)->text(3).length();
    int i = 0;

    while(i < ui->treeWidget->topLevelItemCount())
    {
        if(ui->treeWidget->topLevelItem(i)->text(3).length() != hashLength)
            break;
        i++;
    }

    if(i != ui->treeWidget->topLevelItemCount())
    {
        return nullptr;
    }

    switch (hashLength) {
    case 32:
        return "md5";
    case 40:
        return "sha1";
    case 8:
        return "sfv";
    default:
        return nullptr;
    }
}

void MainWindow::StartHash(bool selected, qint64 &tSize)
{
    ToggleToolbar();

    auto *worker = new HashWorker;
    worker->moveToThread(&workerThread);
    worker->tree = ui->treeWidget;
    worker->totalSize = tSize;
    worker->hashType = ui->cmbHashType->currentIndex();
    worker->selected = selected;

    connect(&workerThread, &QThread::started, worker, &HashWorker::doWork);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &HashWorker::finished, this, &MainWindow::finished);
    connect(worker, &HashWorker::progress, ui->progressBar, &QProgressBar::setValue);
    connect(worker, &HashWorker::hashReady, this, &MainWindow::showHash);
    connect(worker, &HashWorker::error, this, &MainWindow::showError);
    connect(worker, &HashWorker::newSize, this, &MainWindow::updateSize);
    if(Settings::autoScroll())
        connect(worker, &HashWorker::next, this, &MainWindow::handleNext);

    workerThread.start();
}

// --- Remove
void MainWindow::on_actionRemove_triggered()
{
    if(ui->treeWidget->selectedItems().length() == 0)
        return;
    filesCount -= ui->treeWidget->selectedItems().length();
    ui->treeWidget->setUpdatesEnabled(false);
    foreach (QTreeWidgetItem *item, ui->treeWidget->selectedItems()) {
        totalSize -= static_cast<CustomQTreeWidgetItem*>(item)->size;
        filesList.removeOne(item->text(0));
        item->~QTreeWidgetItem();
    }
    ui->treeWidget->setUpdatesEnabled(true);
    statusLabel->setText(QString::number(filesCount) + " File(s) | " + enLocale.formattedDataSize(totalSize));
}

// --- Clear All
void MainWindow::on_actionClear_triggered()
{
    if(ui->treeWidget->topLevelItemCount() == 0)
        return;
    filesCount = 0;
    totalSize = 0;
    filesList.clear();
    ui->treeWidget->clear();
    statusLabel->setText(QString::number(filesCount) + " File(s) | " + enLocale.formattedDataSize(totalSize));
}

// --- Save
void MainWindow::saveButton_clicked()
{
    Save();
}

// --- Calculate Button
void MainWindow::calcButton_clicked()
{
    if(!started)
    {
        qDebug() << "Hash::Start";
        if(ui->treeWidget->topLevelItemCount() == 0)
            return;

        StartHash(false, totalSize);
    }
    else
    {
        qDebug() << "Hash::Interrupt";
        interrupt = true;
        workerThread.requestInterruption();
    }
}

// --- Save As
void MainWindow::saveAsAction_triggered()
{
    Save(true);
}

// --- Restart
void MainWindow::restartAction_triggered()
{
    qDebug() << "Restart::Clicked";
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        ui->treeWidget->topLevelItem(i)->setText(3, "-");
    }
    calcButton_clicked();
}

void MainWindow::Save(bool saveAs)
{
    if(ui->treeWidget->topLevelItemCount() == 0)
            return;

    QString fileExt = GetFileExt();
    if(fileExt.isNull())
    {
        QMessageBox::warning(this, "Save", "The hash list is incomplete.");
        return;
    }

    QString commonRoot = GetCommonRoot();
    if(commonRoot.isEmpty()){
        QMessageBox::warning(this, "Save", "No common root was found.");
        return;
    }

    QString filePath;
    if(ui->treeWidget->topLevelItemCount() > 1)
    {
        filePath = QString("%1/checksums.%2").arg(commonRoot, fileExt);
    }
    else
    {
        filePath = QString("%1/%2.%3").arg(commonRoot, ui->treeWidget->topLevelItem(0)->text(1), fileExt);
    }

    if(saveAs)
    {
        QString filter;
        if(fileExt == "md5") filter = "MD5 Checksum File (*.md5)";
        else if(fileExt == "sha1") filter = "SHA-1 Checksum File (*.sha1)";
        else filter = "CRC32 Checksum File (*.sfv)";

        filePath = QFileDialog::getSaveFileName(this, "Save", filePath, filter);
        if(filePath.isEmpty())
            return;
    }

    int startPos = commonRoot.length() + 1;

    QFile file(filePath);

    if(!saveAs && file.exists())
    {
        QMessageBox::StandardButton answer = QMessageBox::question(this, "Save", QString("%0 already exists.\nDo you want to overwrite it?").arg(filePath));
        if(answer == QMessageBox::No)
            return;
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Save", "Access denied.");
        return;
    }

    bool useBackslash = Settings::useBackslash();
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << "; Generated by RapidSum" << endl;
    QString format(fileExt == "sfv" ? "%2 %1" : fileExt == "md5" && !Settings::md5TwoSpaces() ? "%1 *%2" : "%1  %2");
    for(int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
    {
        stream << format.arg(ui->treeWidget->topLevelItem(i)->text(3),
                  useBackslash ? ui->treeWidget->topLevelItem(i)->text(0).mid(startPos).replace('/','\\') : ui->treeWidget->topLevelItem(i)->text(0).mid(startPos)) << endl;
    }
    file.close();
    QMessageBox::information(this, "Save", QString("Saved in %1").arg(filePath));
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog sd(this);
    sd.exec();
}

void MainWindow::calcAct_triggered()
{
    ui->treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    qint64 total = 0;
    foreach (auto item, ui->treeWidget->selectedItems()) {
        item->setText(3, "-");
        total += static_cast<CustomQTreeWidgetItem*>(item)->size;
    }
    qDebug() << "first total size" << enLocale.formattedDataSize(total);
    StartHash(true, total);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Settings::setLastPath(lastPath);
    QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if(!mimeData->hasUrls()) return;
    qDebug() << "dropEvent::Urls";

    ui->treeWidget->setUpdatesEnabled(false);
    foreach (QUrl url, mimeData->urls())
    {
        QString localUrl = url.toLocalFile();
        bool symlink = QFileInfo(localUrl).isSymLink();
        if(QFileInfo(localUrl).isDir() && !symlink)
        {
            qDebug() << "dropEvent::DIR";

            QDirIterator iterator(localUrl, QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);

            while(iterator.hasNext()){
                QString fileName = iterator.next();
                if(filesList.contains(fileName))
                    continue;
                filesList.push_back(fileName);
                QFile file(fileName);
                if(QFileInfo(file).isSymLink())
                    file.open(QIODevice::ReadOnly);
                qint64 fileSize = file.size();
                totalSize += fileSize;
                QTreeWidgetItem *item = new CustomQTreeWidgetItem(QStringList( {fileName, iterator.fileInfo().fileName(), enLocale.formattedDataSize(fileSize), "-"} ), fileSize);
                ui->treeWidget->addTopLevelItem(item);
                filesCount += 1;
            }
            continue;
        }

        if(filesList.contains(localUrl))
            continue;
        filesList.push_back(localUrl);
        //qint64 fileSize = QFileInfo(fileName).size();
        QFile file(localUrl);
        if(symlink)
            file.open(QIODevice::ReadOnly);
        qint64 fileSize = file.size();
        totalSize += fileSize;
        ui->treeWidget->addTopLevelItem(new CustomQTreeWidgetItem(QStringList( {localUrl, url.fileName(), enLocale.formattedDataSize(fileSize), "-"} ), fileSize));
        filesCount += 1;
    }
    ui->treeWidget->setUpdatesEnabled(true);

    statusLabel->setText(QString::number(filesCount) + " File(s) | " + enLocale.formattedDataSize(totalSize));

}

void MainWindow::on_btnAbout_clicked()
{
    AboutDialog ad(this);
    ad.exec();
}

void MainWindow::showMenu(const QPoint &pos)
{
    if(ui->treeWidget->selectedItems().length() == 0 || started) return;

    QAction *calcAct = new QAction("Calculate", this);
    connect(calcAct, &QAction::triggered, this, &MainWindow::calcAct_triggered);

    QMenu menu(this);
    menu.addAction(calcAct);
    menu.exec(ui->treeWidget->mapToGlobal(pos));
}





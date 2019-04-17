#include "operationdialog.h"
#include "ui_operationdialog.h"
#include "enum.h"


#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolTip>
#include <filesystem>


using namespace std;

OperationDialog::OperationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OperationDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->buttonGroupTarget->setId(ui->radioOK, 0);
    ui->buttonGroupTarget->setId(ui->radioBad, 1);
    ui->buttonGroupTarget->setId(ui->radioAll, 2);
    ui->buttonGroupTarget->setId(ui->radioSelection, 3);

    ui->buttonGroupOp->setId(ui->radioCopy, 0);
    ui->buttonGroupOp->setId(ui->radioMove, 1);
    ui->buttonGroupOp->setId(ui->radioDelete, 2);

    ui->buttonGroupOverwrite->setId(ui->radioAskOverwrite, 0);
    ui->buttonGroupOverwrite->setId(ui->radioYesOverwrite, 1);
    ui->buttonGroupOverwrite->setId(ui->radioNoOverwrite, 2);

    ui->btnCancel->setVisible(false);

    ui->treeWidget->setColumnWidth(0, 150);
    ui->treeWidget->setColumnWidth(1, 150);

}

OperationDialog::~OperationDialog()
{
    workerThread.requestInterruption();
    workerThread.quit();
    workerThread.wait();
    delete ui;
}

void OperationDialog::op_start(QList<QTreeWidgetItem *> files, QString dest, enum Op mode)
{


    QString okText;
    switch (mode) {
    case op_copy: okText = "Copied."; break;
    case op_move: okText = "Moved."; break;
    case op_delete: okText = "Deleted."; break;
    }

    QString rootPath = static_cast<VerifierWindow*>(parent())->rootPath;

    if(mode == op_delete) // DELETE
    {
        error_code err;

        foreach (auto *item, files) {
            QString srcFile = rootPath + item->text(0);

            auto newItem = new QTreeWidgetItem( {item->text(0)} );
            ui->treeWidget->addTopLevelItem(newItem);
            if(ui->autoScrollCheckBox->isChecked())
                ui->treeWidget->scrollToItem(newItem);
            ui->statusLabel->setText(QString("File %1/%2").arg(ui->treeWidget->topLevelItemCount()).arg(totalCount));

            bool res = filesystem::remove(srcFile.toStdWString(), err);

            if(err.value() == 0)
            {
                if(!res)
                {
                    err = make_error_code(errc::no_such_file_or_directory);
                    newItem->setText(2, QString("Error (%1)").arg(QString::fromStdString(err.message()).trimmed()));
                    errorCount++;
                    continue;
                }
                newItem->setText(2, okText);
                successCount++;
            }
            else
            {
                newItem->setText(2, QString("Error (%1)").arg(QString::fromStdString(err.message()).trimmed()));
                errorCount++;
            }
        }

        ui->statusLabel->setText(QString("Done. (%1 Successful | %2 Error)").arg(successCount).arg(errorCount));

    }
    else // COPY or MOVE
    {
        //dontAsk = false;
        //overwrite = false;

        auto opw = new OperationWorker;
        opw->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, opw, &QObject::deleteLater);
        connect(&workerThread, &QThread::started, opw, &OperationWorker::doWork);
        connect(opw, &OperationWorker::finished, this, &OperationDialog::workerFinished);
        connect(opw, &OperationWorker::ask, this, &OperationDialog::askMsg, Qt::BlockingQueuedConnection);
        connect(opw, &OperationWorker::addItem, this, &OperationDialog::addItem);
        connect(opw, &OperationWorker::progress, this, &OperationDialog::handleProgress);
        connect(opw, &OperationWorker::sendResult, this, &OperationDialog::handleResult);
        opw->mode = mode;
        opw->files = files;
        opw->dontAsk = &dontAsk;
        opw->overwrite = &overwrite;
        opw->canceled = &canceled;
        opw->rootPath =rootPath;
        opw->destPath = dest;
        opw->okText = okText;

        workerThread.start();
    }


}


void OperationDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    int ok_count = okFiles->length();
    int bad_count = badFiles->length();
    int selected_count = selectedFiles.length();

    if(ok_count == 0)
        ui->radioOK->setEnabled(false);
    if(bad_count == 0)
        ui->radioBad->setEnabled(false);
    if(selected_count == 0)
        ui->radioSelection->setEnabled(false);
    ui->radioAll->setEnabled(ui->radioOK->isEnabled() && ui->radioBad->isEnabled());

    ui->radioAll->setText(QString("OK && Bad (%1)").arg(ok_count + bad_count));
    ui->radioOK->setText(QString("OK (%1)").arg(ok_count));
    ui->radioBad->setText(QString("Bad (%1)").arg(bad_count));
    ui->radioSelection->setText(QString("Selection (%1)").arg(selected_count));
}

void OperationDialog::on_radioCopy_clicked()
{
    ui->txtDest->setEnabled(true);
    ui->btnBrowse->setEnabled(true);
    ui->overwriteGroupBox->setEnabled(true);
}

void OperationDialog::on_radioMove_clicked()
{
    ui->txtDest->setEnabled(true);
    ui->btnBrowse->setEnabled(true);
    ui->overwriteGroupBox->setEnabled(true);
}

void OperationDialog::on_radioDelete_clicked()
{
    ui->txtDest->setEnabled(false);
    ui->btnBrowse->setEnabled(false);
    ui->overwriteGroupBox->setEnabled(false);
}

void OperationDialog::on_btnBrowse_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select Folder", nullptr);
    if(path.isEmpty())
        return;
    ui->txtDest->setText(path);
}

void OperationDialog::on_btnStart_clicked()
{


    target = ui->buttonGroupTarget->checkedId();
    op = ui->buttonGroupOp->checkedId();

    if(target == -1)
    {
        QToolTip::showText(ui->groupBoxTarget->mapToGlobal(QPoint()),"Select a target");
        return;
    }
    if(op == -1)
    {
        QToolTip::showText(ui->groupBoxOp->mapToGlobal(QPoint()),"Select a operation");
        return;
    }
    if(op < 2) // for COPY or MOVE
    {
        if(ui->txtDest->text().isEmpty())
        {
            QToolTip::showText(ui->btnBrowse->mapToGlobal(QPoint()),"Select a destination");
            return;
        }

        // check dest dir
        if(!QDir(ui->txtDest->text()).exists())
        {
            QMessageBox::warning(this, parentWidget()->windowTitle(), "Destination folder doesn't exist.");
            return;
        }

        destPath = ui->txtDest->text();
        if(destPath.at(destPath.length()-1) != '/')
            destPath.append('/');
    }

    //accept();

    ui->btnCancel->setVisible(true);
    ui->btnStart->setVisible(false);
    ui->frame->setEnabled(false);
    ui->treeWidget->setHeaderHidden(false);

    QList<QTreeWidgetItem*> targetFiles;

    switch (ui->buttonGroupTarget->checkedId()) {
    case 0:
        targetFiles = *okFiles;
        break;
    case 1:
        targetFiles = *badFiles;
        break;
    case 2:
        targetFiles = *okFiles + *badFiles;
        break;
    case 3:
        targetFiles = selectedFiles;
        break;
    }

    totalCount = targetFiles.length();

    switch (ui->buttonGroupOp->checkedId()) {
    case 0:
        op_start(targetFiles, destPath, op_copy);
        break;
    case 1:
        op_start(targetFiles, destPath, op_move);
        break;
    case 2:
        op_start(targetFiles, nullptr, op_delete);
        ui->treeWidget->hideColumn(1);
        break;
    }

    switch (ui->buttonGroupOverwrite->checkedId()) {
    case 0:
        dontAsk = false;
        overwrite = false;
        break;
    case 1:
        dontAsk = true;
        overwrite = true;
        break;
    case 2:
        dontAsk = true;
        overwrite = false;
        break;
    }
}

void OperationDialog::handleProgress(QString val)
{
    ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount()-1)->setText(2, val);
}

void OperationDialog::handleResult(QString msg, int res)
{
    ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount()-1)->setText(2, msg);
    switch (res) {
    case res_success:
        successCount++;
        break;
    case res_skipped:
        skippedCount++;
        break;
    case res_error:
        errorCount++;
        break;
    }
}

void OperationDialog::workerFinished()
{
    qDebug() << "worker::finished";
    workerThread.quit();
    ui->btnCancel->setEnabled(false);
    skippedCount += totalCount - (successCount + skippedCount + errorCount);
    ui->statusLabel->setText(QString("Done. (%1 Successful | %2 Skipped | %3 Error)").arg(successCount).arg(skippedCount).arg(errorCount));
}

void OperationDialog::askMsg(QTreeWidgetItem* item)
{
    auto result = QMessageBox::question(this, "Overwrite", QString("Destination file, %1 already exist.\nDo you want to replace it?").arg(item->text(0)), QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel);
    switch (result) {
    case QMessageBox::YesToAll:
        dontAsk = true;
        [[fallthrough]];
    case QMessageBox::Yes:
        overwrite = true;
        break;
    case QMessageBox::NoToAll:
        dontAsk = true;
        [[fallthrough]];
    case QMessageBox::No:
        overwrite = false;
        break;
    case QMessageBox::Cancel:
        //item->setText(2, "Canceled.");
        canceled = true;
        break;
    }
}

void OperationDialog::addItem(QTreeWidgetItem* item)
{
    ui->treeWidget->addTopLevelItem(item);
    if(ui->autoScrollCheckBox->isChecked())
        ui->treeWidget->scrollToItem(item);
    ui->statusLabel->setText(QString("File %1/%2").arg(ui->treeWidget->topLevelItemCount()).arg(totalCount));
}

void OperationDialog::on_btnCancel_clicked()
{
    ui->btnCancel->setEnabled(false);
    workerThread.requestInterruption();
}

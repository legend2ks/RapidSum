#include "operationworker.h"


#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QThread>

OperationWorker::OperationWorker()
{

}

static DWORD CALLBACK progressRoutine(
  LARGE_INTEGER TotalFileSize,
  LARGE_INTEGER TotalBytesTransferred,
  LARGE_INTEGER StreamSize,
  LARGE_INTEGER StreamBytesTransferred,
  DWORD dwStreamNumber,
  DWORD dwCallbackReason,
  HANDLE hSourceFile,
  HANDLE hDestinationFile,
  LPVOID lpData
)
{
    //qDebug() << "PROGRESS!";
    //qDebug() << "TotalBytesTransferred/TotalFileSize" << TotalBytesTransferred.QuadPart << TotalFileSize.QuadPart;
    //qDebug() << "StreamBytesTransferred/StreamSize" << StreamBytesTransferred.QuadPart << StreamSize.QuadPart;
    auto self = static_cast<OperationWorker*>(lpData);
    if(TotalFileSize.QuadPart)
        emit self->progress( QString::number(TotalBytesTransferred.QuadPart * 100 / TotalFileSize.QuadPart).append('%') );
    //emit self->progress(QString::number(persentDone).append('%'));
    if(self->thread()->isInterruptionRequested())
        return PROGRESS_CANCEL;

    return PROGRESS_CONTINUE;
}

void OperationWorker::doWork()
{
    qDebug() << "OpWorker::Working";

    foreach (auto *item, files) {

        QString srcFile = rootPath + item->text(0);
        QString destFile = destPath + item->text(0);

        emit addItem(new QTreeWidgetItem( {item->text(0), destFile} ));

        if(thread()->isInterruptionRequested())
        {
            emit sendResult("Skipped (Canceled.)", res_skipped);
            emit finished();
            return;
        }

        if(!QFile::exists(srcFile))
        {
            emit sendResult("Error (Source file doesn't exist.)", res_error);
            continue;
        }
        if(QDir(destFile).exists())
        {
            emit sendResult("Error (There is already a folder with the same name.)", res_error);
            continue;
        }
        if(QFile::exists(destFile))
        {
            if(!*dontAsk)
            {
                emit ask(item);
                if(*canceled)
                {
                    emit sendResult("Skipped (Canceled.)", res_skipped);
                    emit finished();
                    return;
                }
            }
            if(!*overwrite)
            {
                emit sendResult("Skipped (Destination file exists.)", res_skipped);
                continue;
            }
        }
        if(QDir tDir = QDir(destFile); !tDir.cdUp())
        {
            if(!tDir.mkpath(QFileInfo(destFile).path()))
            {
                emit sendResult("Error (Access denied.)", res_error);
                continue;
            }
        }

        BOOL res = FALSE;

        switch (mode) {
        case op_copy:
            qDebug() << "do copy";
            res = CopyFileEx((LPCWSTR)srcFile.utf16(), (LPCWSTR)destFile.utf16(), &progressRoutine, this, nullptr, NULL);
            break;
        case op_move:
            qDebug() << "do move";
            res = MoveFileWithProgress((LPCWSTR)srcFile.utf16(), (LPCWSTR)destFile.utf16(), &progressRoutine, this, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
            break;
        }

        if(!res)
        {
            if(thread()->isInterruptionRequested())
            {
                emit sendResult("Skipped (Canceled.)", res_skipped);
                emit finished();
                return;
            }
            emit sendResult("Error (Access Denied?)", res_error);
        }
        else
        {
            emit sendResult(okText, res_success);
        }
    }

    /*if(!result)
        qDebug() << "error:" << toString(HRESULT_FROM_WIN32(GetLastError()));*/

    emit finished();
}


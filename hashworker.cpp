#include "hashworker.h"
#include "settings.h"
#include "customqtreewidgetitem.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QFile>
#include <QThread>
#include <QElapsedTimer>
#include <QtZlib/zlib.h>

HashWorker::HashWorker()
= default;

HashWorker::~HashWorker()
= default;

void HashWorker::doWork()
{
    qDebug() << "HashWorker::Started";

    switch (hashType) {
        case 0:
            doMD5SHA1(QCryptographicHash::Md5);
            break;
        case 1:
            doMD5SHA1(QCryptographicHash::Sha1);
            break;
        case 2:
            doCRC32();
            break;
        default:
            return;
    }

    emit finished();
}

void HashWorker::doMD5SHA1(QCryptographicHash::Algorithm algo)
{
    QCryptographicHash hasher(algo);
    int bufferSize = Settings::bufferSize();
    QByteArray buffer;
    qint64 fileSize;
    qint64 totalBytesRead = 0;

    int hashLength = algo == QCryptographicHash::Md5 ? 32 : 40;
    qint64 *tSize = verifyMode ? &fileSize : &totalSize;


    for(int i = 0; i < tree->topLevelItemCount(); i++)
    {
        if(selected)
        {
            if(!tree->topLevelItem(i)->isSelected()) continue;
        }
        else
        {
            if(tree->topLevelItem(i)->text(3).length() == hashLength)
            {
                totalBytesRead += static_cast<CustomQTreeWidgetItem*>(tree->topLevelItem(i))->size;
                continue;
            }
        }
        emit next(i);

        QFile file(verifyMode ? rootPath + tree->topLevelItem(i)->text(0) : tree->topLevelItem(i)->text(0));
        if(!file.exists())
        {
            emit error("File doesn't exist", i);
            continue;
        }

        if(!file.open(QFile::ReadOnly))
        {
            emit error("Access denied", i);
            continue;
        }
        fileSize = file.size();
        if(verifyMode)
            totalBytesRead = 0;
        else if(auto *item = static_cast<CustomQTreeWidgetItem*>(tree->topLevelItem(i)); fileSize != item->size)
        {
            qDebug() << "File size mismatch!";
            qint64 diff = fileSize - item->size;
            item->size = fileSize;
            emit newSize(diff, i);
            totalSize += diff;
        }
        bool readError = false;
        qint64 totalBytesReadAfter = totalBytesRead + fileSize;

        while(!file.atEnd()){
            if(thread()->isInterruptionRequested())
                return;
            buffer = file.read(bufferSize);
            if(file.error())
            {
                readError = true;
                emit error("Access denied", i);
                break;
            }
            hasher.addData(buffer);
            totalBytesRead += buffer.length();
            emit progress((totalBytesRead * 100 / *tSize), i);
        }
        file.close();
        if(readError) {
            if(!verifyMode)
                totalBytesRead = totalBytesReadAfter;
            continue;
        }

        emit hashReady(i, hasher.result().toHex());
        hasher.reset();
    }
}

void HashWorker::doCRC32()
{
    QByteArray buffer;
    qint64 fileSize;
    qint64 totalBytesRead = 0;
    int bufferSize = Settings::bufferSize();
    int hashLength = 8;
    qint64 *tSize = verifyMode ? &fileSize : &totalSize;

    for(int i = 0; i < tree->topLevelItemCount(); i++)
    {
        if(selected)
        {
            if(!tree->topLevelItem(i)->isSelected()) continue;
        }
        else
        {
            if(tree->topLevelItem(i)->text(3).length() == hashLength)
            {
                totalBytesRead += static_cast<CustomQTreeWidgetItem*>(tree->topLevelItem(i))->size;
                continue;
            }
        }
        emit next(i);

        QFile file(verifyMode ? rootPath + tree->topLevelItem(i)->text(0) : tree->topLevelItem(i)->text(0));
        if(!file.exists())
        {
            emit error("File doesn't exist", i);
            continue;
        }

        if(!file.open(QFile::ReadOnly))
        {
            emit error("Access denied", i);
            continue;
        }
        fileSize = file.size();
        if(verifyMode)
            totalBytesRead = 0;
        else if(auto *item = static_cast<CustomQTreeWidgetItem*>(tree->topLevelItem(i)); fileSize != item->size)
        {
            qDebug() << "File size mismatch!";
            qint64 diff = fileSize - item->size;
            item->size = fileSize;
            emit newSize(diff, i);
            totalSize += diff;
        }

        unsigned long crc = crc32(0L, nullptr, 0);
        bool readError = false;
        qint64 totalBytesReadAfter = totalBytesRead + fileSize;

        while(!file.atEnd())
        {
            if(thread()->isInterruptionRequested())
                return;
            buffer = file.read(bufferSize);
            if(file.error())
            {
                readError = true;
                emit error("Access denied", i);
                break;
            }
            crc = crc32(crc, (unsigned char*)buffer.data(), z_uInt(buffer.length()));
            totalBytesRead += buffer.length();
            emit progress((totalBytesRead * 100 / *tSize), i);
        }
        file.close();
        if(readError) {
            if(!verifyMode)
                totalBytesRead = totalBytesReadAfter;
            continue;
        }

        emit hashReady(i, QByteArray::number(qulonglong(crc),16).rightJustified(8, '0'));
    }
}


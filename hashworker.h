#ifndef HASHWORKER_H
#define HASHWORKER_H

#include <QObject>
#include <QTreeWidget>
#include <QProgressBar>
#include <QCryptographicHash>

class HashWorker : public QObject
{
    Q_OBJECT

public:
    HashWorker();
    ~HashWorker();
    QTreeWidget *tree{};
    qint64 totalSize{};
    int hashType{};
    bool selected = false;
    bool verifyMode = false;
    QString *rootPath{};

signals:
    void hashReady(int, QByteArray);
    void progress(int, int);
    void finished();
    void error(QString, int);
    void newSize(qint64 diff , int index);
    void next(int);

public slots:
    void doWork();
    void doMD5SHA1(QCryptographicHash::Algorithm);
    void doCRC32();
};

#endif // HASHWORKER_H

#ifndef OPERATIONWORKER_H
#define OPERATIONWORKER_H

#include "enum.h"
#include <QObject>
#include <QTreeWidgetItem>
#include <Windows.h>

class OperationWorker : public QObject
{
    Q_OBJECT

public:
    OperationWorker();
    QList<QTreeWidgetItem *> files;
    QString rootPath;
    QString destPath;
    Op mode;
    bool *dontAsk;
    bool *overwrite;
    bool *canceled;
    QString okText;

signals:
    void finished();
    void progress(QString);
    void ask(QTreeWidgetItem*);
    void addItem(QTreeWidgetItem*);
    void sendResult(QString, int);

public slots:
    void doWork();
};

#endif // OPERATIONWORKER_H

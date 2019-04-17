#ifndef OPERATIONDIALOG_H
#define OPERATIONDIALOG_H

#include "verifierwindow.h"
#include "operationworker.h"
#include "enum.h"
#include <QDialog>
#include <QProgressBar>
#include <QTreeWidget>



namespace Ui {
class OperationDialog;
}

class OperationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OperationDialog(QWidget *parent = nullptr);
    ~OperationDialog();

    QString destPath;
    int target;
    int op;

    QList<QTreeWidgetItem*> *okFiles;
    QList<QTreeWidgetItem*> *badFiles;
    QList<QTreeWidgetItem*> selectedFiles;

private:
    Ui::OperationDialog *ui;

    bool firstTime = true;
    QThread workerThread;
    void op_start(QList<QTreeWidgetItem*>, QString, enum Op);
    bool dontAsk = false;
    bool overwrite = false;
    bool canceled = false;
    int totalCount;
    int successCount = 0;
    int errorCount = 0;
    int skippedCount = 0;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
private slots:
    void on_radioCopy_clicked();
    void on_radioMove_clicked();
    void on_radioDelete_clicked();
    void on_btnBrowse_clicked();
    void on_btnStart_clicked();
    void handleProgress(QString);
    void handleResult(QString, int);
    void workerFinished();
    void askMsg(QTreeWidgetItem *item);
    void addItem(QTreeWidgetItem*);

    void on_btnCancel_clicked();

signals:
    void startCopy(QString, QString);
    void startMove(QString, QString);

};

#endif // OPERATIONDIALOG_H

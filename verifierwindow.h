#ifndef VERIFIERWINDOW_H
#define VERIFIERWINDOW_H

#include "settings.h"
#include <QLabel>
#include <QMainWindow>
#include <QThread>
#include <QTreeWidgetItem>

namespace Ui {
class VerifierWindow;
}

class VerifierWindow : public QMainWindow
{
    Q_OBJECT

    QThread workerThread;
    int filesCount = 0;
    int checkedCount = 1;
    int okCount = 0;
    int badCount = 0;
    int missingCount = 0;
    int hashType{};
    //QString rootPath;
    QIcon icon[4];
    QList<QTreeWidgetItem*> okFiles;
    QList<QTreeWidgetItem*> badFiles;
    QList<QTreeWidgetItem*> missingFiles;

public:
    explicit VerifierWindow(QWidget *parent = nullptr);
    ~VerifierWindow();

    QString rootPath;

private:
    Ui::VerifierWindow *ui;
    bool firstTime = true;
    bool ReadFile();
    void Verify();
    QLabel *statusLabel;


private slots:
    void finished();
    void showResult(int, const QByteArray&);
    void showProgress(int, int);
    void showError(const QString&, int);
    void on_checkBoxOK_stateChanged(int arg1);
    void on_checkBoxBad_stateChanged(int arg1);
    void on_checkBoxMissing_stateChanged(int arg1);
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionOperation_triggered();


    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;

};

#endif // VERIFIERWINDOW_H

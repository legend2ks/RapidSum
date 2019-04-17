#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settings.h"
#include <QLabel>
#include <QMainWindow>
#include <QThread>
#include <QToolButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QThread workerThread;
    qint64 totalSize = 0;
    QToolButton *saveButton;
    QToolButton *calcButton;
    QLabel *statusLabel;
    int filesCount = 0;
    int completedCount = 0;
    QList<QString> filesList;
    QString lastPath;
    bool started = false;
    bool interrupt = false;
    QIcon startStopIcon[2];

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void ToggleToolbar();
    void Save(bool saveAs = false);
    QString GetCommonRoot();
    QString GetFileExt();
    void StartHash(bool, qint64 &);

private slots:
    void on_actionAdd_Files_triggered();
    void on_chkFullPath_stateChanged(int arg1);
    void saveButton_clicked();
    void calcButton_clicked();
    void saveAsAction_triggered();
    void restartAction_triggered();

    void showHash(int, const QByteArray&);
    void showError(const QString&, int);
    void updateSize(qint64, int);
    void finished();
    void handleNext(int);

    void on_actionAdd_Folder_triggered();
    void on_actionRemove_triggered();
    void on_actionClear_triggered();
    void on_actionSettings_triggered();
    void calcAct_triggered();
    void on_btnAbout_clicked();

    void showMenu(const QPoint &);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

};

#endif // MAINWINDOW_H

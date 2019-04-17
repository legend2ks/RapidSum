#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <settings.h>
#include <qt_windows.h>


namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private:
    Ui::SettingsDialog *ui;
    void LoadSettings();
    bool RegisterFileAssociation(QString, int);
    bool CheckResult(LSTATUS);

private slots:
    void SaveSettings();
    void ShowExample();
    void on_btnMd5_clicked();
    void on_btnSfv_clicked();
    void on_btnSha1_clicked();
};

#endif // SETTINGSDIALOG_H

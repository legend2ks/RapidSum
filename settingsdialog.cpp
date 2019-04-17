#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QDebug>
#include <QDir>
#include <QIntValidator>
#include <QLayout>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    ui->lineEditBufferSize->setValidator(new QIntValidator(4096, 3145728, this));

    LoadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::LoadSettings()
{
    ui->lineEditBufferSize->setText(QString::number(Settings::bufferSize()));
    ui->checkBoxTwoSpaces->setChecked(Settings::md5TwoSpaces());
    ui->checkBoxUseBackslash->setChecked(Settings::useBackslash());
    ui->autoScrollCheckBox->setChecked(Settings::autoScroll());
    ShowExample();
}

bool SettingsDialog::RegisterFileAssociation(QString ext, int icon)
{
    QString key = QString("RapidSum.%1.Checksum.File").arg(ext.toUpper());

    LSTATUS res;

    res = RegSetKeyValueA(HKEY_CLASSES_ROOT, QString(".%1").arg(ext).toStdString().data(),
                          nullptr, REG_SZ, key.toStdString().data(), static_cast<DWORD>(key.size()));
    if(!CheckResult(res)) return false;

    QString iconValue = QString("%1,%2").arg(QDir::toNativeSeparators(qApp->applicationFilePath())).arg(icon);
    res = RegSetKeyValueA(HKEY_CLASSES_ROOT, QString("%1\\DefaultIcon").arg(key).toStdString().data(),
                          nullptr,REG_SZ, iconValue.toStdString().data(), static_cast<DWORD>(iconValue.size()));
    if(!CheckResult(res)) return false;

    QString cmdValue = QString("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(qApp->applicationFilePath())).arg("%1");
    res = RegSetKeyValueA(HKEY_CLASSES_ROOT, QString("%1\\shell\\open\\command").arg(key).toStdString().data(),
                          nullptr, REG_SZ, cmdValue.toStdString().data(), static_cast<DWORD>(cmdValue.size()));
    if(!CheckResult(res)) return false;

    res = RegDeleteKeyA(HKEY_CURRENT_USER, QString("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.%1\\UserChoice").arg(ext).toStdString().data());
    if(!CheckResult(res)) return false;

    return true;
}

bool SettingsDialog::CheckResult(LSTATUS res)
{
    switch (res) {
    case ERROR_SUCCESS:
    case ERROR_FILE_NOT_FOUND:
        return true;
    case ERROR_ACCESS_DENIED:
        QMessageBox::warning(this, "Access denied", "You don't have administrator rights.");
        return false;
    default:
        QMessageBox::warning(this, "Error", "Unknown error.");
        return false;
    }
}

void SettingsDialog::ShowExample()
{
    ui->labelExample->setText(QString("0123456789abcdef0123456789abcdef%1dir%2filename.ext")
                              .arg(ui->checkBoxTwoSpaces->isChecked() ? "  " : " *")
                              .arg(ui->checkBoxUseBackslash->isChecked() ? '\\' : '/'));
}

void SettingsDialog::SaveSettings()
{
    if(ui->lineEditBufferSize->text().toInt() < 4096) ui->lineEditBufferSize->setText("4096");
    Settings::setBufferSize(ui->lineEditBufferSize->text().toInt());
    Settings::setMd5TwoSpaces(ui->checkBoxTwoSpaces->isChecked());
    Settings::setUseBackslash(ui->checkBoxUseBackslash->isChecked());
    Settings::setAutoScroll(ui->autoScrollCheckBox->isChecked());
    accept();

}

void SettingsDialog::on_btnMd5_clicked()
{
    if(RegisterFileAssociation("md5", 1))
    {
        ui->btnMd5->setText(".md5 ✓");
        ui->btnMd5->setEnabled(false);
    }
}

void SettingsDialog::on_btnSfv_clicked()
{
    if(RegisterFileAssociation("sfv", 2))
    {
        ui->btnSfv->setText(".sfv ✓");
        ui->btnSfv->setEnabled(false);
    }
}

void SettingsDialog::on_btnSha1_clicked()
{
    if(RegisterFileAssociation("sha1", 3))
    {
        ui->btnSha1->setText(".sha1 ✓");
        ui->btnSha1->setEnabled(false);
    }
}

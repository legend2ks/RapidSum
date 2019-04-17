#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QDebug>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    #if _WIN64
        ui->txtVersion->setText(QString("v%0 (64bit)").arg(qApp->applicationVersion()));
    #else //if _WIN32
        ui->txtVersion->setText(QString("v%0 (32bit)").arg(qApp->applicationVersion()));
    #endif

}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_btnOK_clicked()
{
    this->close();
}

void AboutDialog::on_btnQt_clicked()
{
    QApplication::aboutQt();
}

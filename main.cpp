#include "mainwindow.h"
#include "verifierwindow.h"
#include <qt_windows.h>
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QMessageBox>
#include <QDir>

void RegisterFileAssociation(QString ext, int icon)
{
    QString key = QString("RapidSum.%1.Checksum.File").arg(ext.toUpper());
    RegSetKeyValueA(HKEY_CLASSES_ROOT, QString(".%1").arg(ext).toStdString().data(),
                          nullptr, REG_SZ, key.toStdString().data(), static_cast<DWORD>(key.size()));
    QString iconValue = QString("%1,%2").arg(QDir::toNativeSeparators(qApp->applicationFilePath())).arg(icon);
    RegSetKeyValueA(HKEY_CLASSES_ROOT, QString("%1\\DefaultIcon").arg(key).toStdString().data(),
                          nullptr,REG_SZ, iconValue.toStdString().data(), static_cast<DWORD>(iconValue.size()));
    QString cmdValue = QString("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(qApp->applicationFilePath())).arg("%1");
    RegSetKeyValueA(HKEY_CLASSES_ROOT, QString("%1\\shell\\open\\command").arg(key).toStdString().data(),
                          nullptr, REG_SZ, cmdValue.toStdString().data(), static_cast<DWORD>(cmdValue.size()));
    RegDeleteKeyA(HKEY_CURRENT_USER, QString("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.%1\\UserChoice").arg(ext).toStdString().data());
}

int main(int argc, char *argv[])
{

    QCoreApplication::libraryPaths();
    QApplication a(argc, argv);

    if(QApplication::arguments().length() == 2)
    {
        if(QApplication::arguments().at(1).compare("-register", Qt::CaseInsensitive) == 0) {
            RegisterFileAssociation("md5", 1);
            RegisterFileAssociation("sfv", 2);
            RegisterFileAssociation("sha1", 3);
            return 0;
        }

        VerifierWindow w;
        w.show();
        return QApplication::exec();
    }
    
    MainWindow w;
    w.show();
    return QApplication::exec();
}



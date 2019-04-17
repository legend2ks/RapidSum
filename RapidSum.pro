#-------------------------------------------------
#
# Project created by QtCreator 2018-12-09T16:32:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RapidSum
TEMPLATE = app
VERSION = 1.1.5.0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17
CONFIG += static

LIBS += -ladvapi32

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    hashworker.cpp \
    customqtreewidgetitem.cpp \
    verifierwindow.cpp \
    settings.cpp \
    settingsdialog.cpp \
    aboutdialog.cpp \
    operationdialog.cpp \
    operationworker.cpp

HEADERS += \
        mainwindow.h \
    hashworker.h \
    customqtreewidgetitem.h \
    verifierwindow.h \
    settings.h \
    settingsdialog.h \
    aboutdialog.h \
    operationdialog.h \
    operationworker.h \
    enum.h

FORMS += \
        mainwindow.ui \
    verifierwindow.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    operationdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

RC_ICONS = appicon.ico md5.ico sfv.ico sha1.ico


DISTFILES +=

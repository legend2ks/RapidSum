#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QDebug>


class Settings
{


public:
    static void init();
    static int bufferSize();
    static void setBufferSize(int bufferSize);

    static bool md5TwoSpaces();
    static void setMd5TwoSpaces(bool md5TwoSpaces);

    static bool useBackslash();
    static void setUseBackslash(bool useBackslash);

    static QString lastPath();
    static void setLastPath(const QString &lastPath);

    static bool autoScroll();
    static void setAutoScroll(bool value);

private:
    Settings();
    static QSettings *_settings;
};

#endif // SETTINGS_H

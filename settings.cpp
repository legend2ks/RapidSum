#include "settings.h"

QSettings *Settings::_settings;

void Settings::init()
{
    _settings = new QSettings("RapidSum", "RapidSum");
}


int Settings::bufferSize()
{
    int size = _settings->value("bufferSize", 16384).toInt();
    if(size < 4096)
        size = 4096;
    else if(size > 3145728)
        size = 3145728;
    return size;
}

void Settings::setBufferSize(int bufferSize)
{
    _settings->setValue("bufferSize", bufferSize);
}

bool Settings::md5TwoSpaces()
{
    return _settings->value("md5TwoSpaces", false).toBool();
}

void Settings::setMd5TwoSpaces(bool md5TwoSpaces)
{
    _settings->setValue("md5TwoSpaces", md5TwoSpaces);
}

bool Settings::useBackslash()
{
    return _settings->value("useBackslash", true).toBool();
}

void Settings::setUseBackslash(bool useBackslash)
{
    _settings->setValue("useBackslash", useBackslash);
}

QString Settings::lastPath()
{
    return _settings->value("lastPath", QString()).toString();
}

void Settings::setLastPath(const QString &lastPath)
{
    _settings->setValue("lastPath", lastPath);
}

bool Settings::autoScroll()
{
    return _settings->value("autoScroll", false).toBool();
}

void Settings::setAutoScroll(bool autoScroll)
{
    _settings->setValue("autoScroll", autoScroll);
}




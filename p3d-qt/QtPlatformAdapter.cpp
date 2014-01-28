#include "QtPlatformAdapter.h"
#include <QFile>
#include <QDebug>
#include <cstdio>

QtPlatformAdapter::QtPlatformAdapter(QObject *parent) :
    QObject(parent)
{
}

QtPlatformAdapter::~QtPlatformAdapter()
{
}

const char *QtPlatformAdapter::loadAsset(const char *filename, size_t *size)
{
#ifdef Q_OS_ANDROID
    QFile file(QStringLiteral("assets:/") + filename);
#else
    QFile file(filename);
#endif
    if (!file.open(QIODevice::ReadOnly))
        return 0;
    QByteArray data = file.readAll();
    char *result;
    if(size)
    {
        *size = data.size();
        result = new char[data.size()];
        memcpy(result, data.constData(), data.size());
    }
    else
    {
        result = new char[data.size() + 1];
        memcpy(result, data.constData(), data.size() + 1);
    }
    return result;
}

void QtPlatformAdapter::logTag(PlatformAdapter::LogLevel level, const char *tag, const char *format, va_list args)
{
    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, args);

    switch(level)
    {
    case LOG_INFO:
        qDebug() << "I" << tag << buf;
        break;
    case LOG_DEBUG:
        qDebug() << "D" << tag << buf;
        break;
    case LOG_WARN:
        qWarning() << "W" << tag << buf;
        break;
    case LOG_ERROR:
        qWarning() << "E" << tag << buf;
        break;
    case LOG_FATAL:
        qFatal("F %s %s", tag, buf);
        break;
    default:
        qDebug() << "U" << tag << buf;
        break;
    }
}



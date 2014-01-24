#include "QtPlatformAdapter.h"
#include <QFile>

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



#ifndef QTPLATFORMADAPTER_H
#define QTPLATFORMADAPTER_H

#include <QObject>
#include "PlatformAdapter.h"

class QtPlatformAdapter : public QObject, public PlatformAdapter
{
    Q_OBJECT
public:
    explicit QtPlatformAdapter(QObject *parent = 0);
    virtual ~QtPlatformAdapter();

    virtual const char* loadAsset(const char *filename, size_t *size);
    virtual void logTag(P3dLogger::Level level, const char* tag, const char* format, va_list args);

signals:

public slots:

};

#endif // QTPLATFORMADAPTER_H

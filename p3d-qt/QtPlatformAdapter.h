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

signals:

public slots:

};

#endif // QTPLATFORMADAPTER_H

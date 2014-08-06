#ifndef QTPLATFORMADAPTER_H
#define QTPLATFORMADAPTER_H

#include "PlatformAdapter.h"
#include <QObject>
#include <QMap>

class QOpenGLTexture;

class QtPlatformAdapter : public QObject, public PlatformAdapter
{
    Q_OBJECT
public:
    explicit QtPlatformAdapter(QObject *parent = 0);
    virtual ~QtPlatformAdapter();

    virtual uint32_t loadTexture(const char *name);
    virtual void deleteTexture(uint32_t textureId);
    virtual const char* loadAsset(const char *filename, size_t *size);
    virtual void logTag(P3dLogger::Level level, const char* tag, const char* format, va_list args);

signals:

public slots:

private:
    QMap<int, QOpenGLTexture*> m_textures;
};

#endif // QTPLATFORMADAPTER_H

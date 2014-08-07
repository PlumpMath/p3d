#ifndef QTPLATFORMADAPTER_H
#define QTPLATFORMADAPTER_H

#include "PlatformAdapter.h"
#include <QObject>
#include <QMap>

class QOpenGLTexture;
class QNetworkAccessManager;
class QNetworkReply;

class QtPlatformAdapter : public QObject, public PlatformAdapter
{
    Q_OBJECT
public:
    explicit QtPlatformAdapter(QObject *parent = 0);
    virtual ~QtPlatformAdapter();

    virtual void loadTexture(const char *name, std::function<void(uint32_t)> callback);
    virtual void deleteTexture(uint32_t textureId);
    virtual void cancelTextureLoads();
    virtual const char* loadAsset(const char *filename, size_t *size);
    virtual void logTag(P3dLogger::Level level, const char* tag, const char* format, va_list args);

signals:

public slots:

private:
    QNetworkAccessManager* m_NetMgr;
    QMap<int, QOpenGLTexture*> m_textures;
    QList<QNetworkReply*> m_pendingTextures;
};

#endif // QTPLATFORMADAPTER_H

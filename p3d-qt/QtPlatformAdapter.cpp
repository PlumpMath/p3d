#include "QtPlatformAdapter.h"
#include <QFile>
#include <QDebug>
#include <QOpenGLTexture>
#include <cstdio>

QtPlatformAdapter::QtPlatformAdapter(QObject *parent) :
    QObject(parent)
{
}

QtPlatformAdapter::~QtPlatformAdapter()
{
}

uint32_t QtPlatformAdapter::loadTexture(const char *name)
{
    QOpenGLTexture *texture = new QOpenGLTexture(QImage(name).mirrored());
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_textures.insert(texture->textureId(), texture);
    return texture->textureId();
}

void QtPlatformAdapter::deleteTexture(uint32_t textureId)
{
    QOpenGLTexture *texture = m_textures.take(textureId);
    if(texture)
    {
        delete texture;
    }
}

const char *QtPlatformAdapter::loadAsset(const char *filename, size_t *size)
{
    QFile file(QStringLiteral(":/") + filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        P3D_LOGE("Can't open asset: %s", filename);
        return 0;
    }
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

void QtPlatformAdapter::logTag(P3dLogger::Level level, const char *tag, const char *format, va_list args)
{
    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, args);

    switch(level)
    {
    case P3dLogger::Level::LOG_VERBOSE:
        qDebug() << "V" << tag << buf;
        break;
    case P3dLogger::Level::LOG_DEBUG:
        qDebug() << "D" << tag << buf;
        break;
    case P3dLogger::Level::LOG_INFO:
        qDebug() << "I" << tag << buf;
        break;
    case P3dLogger::Level::LOG_WARN:
        qWarning() << "W" << tag << buf;
        break;
    case P3dLogger::Level::LOG_ERROR:
        qWarning() << "E" << tag << buf;
        break;
    case P3dLogger::Level::LOG_FATAL:
        qFatal("F %s %s", tag, buf);
        break;
    default:
        qDebug() << "U" << tag << buf;
        break;
    }
}



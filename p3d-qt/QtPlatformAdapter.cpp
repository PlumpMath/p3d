#include "QtPlatformAdapter.h"
#include <QFile>
#include <QDebug>
#include <QOpenGLTexture>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <cstdio>

static P3dLogger logger("qt.QtPlatformAdapter", P3dLogger::LOG_DEBUG);

QtPlatformAdapter::QtPlatformAdapter(QObject *parent) :
    QObject(parent)
{
    m_NetMgr = nullptr;
}

QtPlatformAdapter::~QtPlatformAdapter()
{
    if(m_NetMgr)
    {
        m_NetMgr->deleteLater();
    }
}

void QtPlatformAdapter::loadTexture(const char *name, std::function<void(uint32_t)> callback)
{
    //TODO: handle threading better
    QUrl url(name);
    if(url.isLocalFile())
    {
        QOpenGLTexture *texture = new QOpenGLTexture(QImage(url.toLocalFile()).mirrored());
        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_textures.insert(texture->textureId(), texture);
        callback(texture->textureId());
    }
    else
    {
        if(!m_NetMgr)
        {
            m_NetMgr = new QNetworkAccessManager();
        }
        QNetworkReply* reply = m_NetMgr->get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, [=]() {
            logger.debug("got image data: %s", url.toString().toUtf8().constData());
            QByteArray bytes = reply->readAll();
            logger.debug(" %d bytes", bytes.length());
            reply->deleteLater();

            QOpenGLTexture *texture = new QOpenGLTexture(QImage::fromData(bytes));
            texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
            texture->setMagnificationFilter(QOpenGLTexture::Linear);
            m_textures.insert(texture->textureId(), texture);
            callback(texture->textureId());
            m_pendingTextures.removeOne(reply);
        });
        m_pendingTextures.append(reply);
    }
}

void QtPlatformAdapter::deleteTexture(uint32_t textureId)
{
    QOpenGLTexture *texture = m_textures.take(textureId);
    if(texture)
    {
        delete texture;
    }
}

void QtPlatformAdapter::cancelTextureLoads()
{
    logger.debug("cancel textures");
    for(QNetworkReply* reply: m_pendingTextures)
    {
        reply->disconnect(SIGNAL(finished()));
        reply->abort();
        reply->deleteLater();
    }
    m_pendingTextures.clear();
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



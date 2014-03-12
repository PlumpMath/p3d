#include "QmlAppViewer.h"
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <QOpenGLContext>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QFile>

#include "ModelLoader.h"
#include "P3dViewer.h"
#include "CameraNavigation.h"
#include "QtPlatformAdapter.h"

QmlAppViewer::QmlAppViewer(QObject *parent) :
    QtQuick2ControlsApplicationViewer(parent)
{
    qmlRegisterUncreatableType<QmlAppViewer>("p3d.p3dviewer", 1, 0, "Viewer", "Uncreatable");
    engine.rootContext()->setContextProperty("viewer", this);

    connect(this, SIGNAL(windowReady()), SLOT(onWindowReady()));
    m_P3dViewer = new P3dViewer(new QtPlatformAdapter());
    m_NetMgr = new QNetworkAccessManager(this);
    m_NetInfoReply = 0;
    m_NetDataReply = 0;
    m_ModelState = MS_NONE;
    m_ClearModel = false;
}

QmlAppViewer::~QmlAppViewer()
{
    delete m_P3dViewer;
}

void QmlAppViewer::setModelState(ModelState newValue)
{
    if(newValue != m_ModelState)
    {
        m_ModelState = newValue;
        emit modelStateChanged();
    }
}

void QmlAppViewer::loadModel(const QUrl &model)
{
    QString fileName = model.fileName();
    QString path = model.isLocalFile() ? model.toLocalFile() : model.path();

    setModelState(MS_LOADING);
    if(fileName.endsWith(".blend"))
    {
        m_extension = ".blend";
        qDebug() << "found a blend file: " << path;
        // loading a .blend
        QFile file(path);
        if(!file.exists())
        {
            qWarning() << "File doesn't exist: " << model;
            return;
        }
        //P3D_LOGD("blender geom size %d", m_BlendData->vertbytes + m_BlendData->facebytes);

        m_ModelData = path.toLocal8Bit();
        setModelState(MS_PROCESSING);
        window->update();

        return;

    }

    if(fileName.endsWith(".bin"))
    {
        m_extension = ".bin";
        // this is a local binary file
        QFile file(path);
        if(!file.exists())
        {
            qWarning() << "File doesn't exist:" << model;
            return;
        }
        file.open(QFile::ReadOnly);
        m_ModelData = file.readAll();
        qDebug() << "loaded" << m_ModelData.size() << "bytes";
        setModelState(MS_PROCESSING);
        window->update();
        return;
    }
    if(m_NetInfoReply)
    {
        m_NetInfoReply->abort();
        m_NetInfoReply->deleteLater();
        m_NetInfoReply = 0;
    }

    if(m_NetDataReply)
    {
        m_NetDataReply->abort();
        m_NetDataReply->deleteLater();
        m_NetDataReply = 0;
    }

    m_ModelData.clear();

    m_NetInfoReply = m_NetMgr->get(QNetworkRequest(QUrl("http://p3d.in/api/viewer_models/" + fileName)));
    connect(m_NetInfoReply, SIGNAL(finished()), SLOT(onModelInfoReplyDone()));
}

void QmlAppViewer::clearModel()
{
    m_ClearModel = true;
    window->update();
}

void QmlAppViewer::startRotateCamera(float x, float y)
{
    m_P3dViewer->cameraNavigation()->startRotate(x, y);
}

void QmlAppViewer::rotateCamera(float x, float y)
{
    m_P3dViewer->cameraNavigation()->rotate(x, y);
    window->update();
}

void QmlAppViewer::resetCamera()
{
    m_P3dViewer->cameraNavigation()->reset();
    window->update();
}

void QmlAppViewer::onWindowReady()
{
    connect(window, SIGNAL(beforeRendering()), SLOT(onGLRender()), Qt::DirectConnection);
    connect(window, SIGNAL(sceneGraphInitialized()), SLOT(onGLInit()), Qt::DirectConnection);
    connect(window, SIGNAL(widthChanged(int)), SLOT(onGLResize()), Qt::DirectConnection);
    connect(window, SIGNAL(heightChanged(int)), SLOT(onGLResize()), Qt::DirectConnection);
    window->setClearBeforeRendering(false);
}

void QmlAppViewer::onGLInit()
{
    qDebug() << "GL ctx:" << QOpenGLContext::currentContext();
    m_P3dViewer->onSurfaceCreated();
    onGLResize();
}

void QmlAppViewer::onGLResize()
{
    m_P3dViewer->onSurfaceChanged(window->width(), window->height());

    // workaround for qt/xorg weirdness
    QTimer::singleShot(25, window, SLOT(update()));
}

void QmlAppViewer::onGLRender()
{
    //TODO: threading
    if(!m_ModelData.isEmpty())
    {
        setModelState(MS_PROCESSING);
        m_P3dViewer->loadModel(m_ModelData.constData(), m_ModelData.size(), m_extension.toLocal8Bit().constData());
        m_ModelData.clear();
        setModelState(MS_READY);
    }

    if(m_ClearModel)
    {
        m_ClearModel = false;
        m_P3dViewer->clearModel();
        setModelState(MS_NONE);
    }
    window->resetOpenGLState();
    m_P3dViewer->drawFrame();
}

void QmlAppViewer::onModelInfoReplyDone()
{
    m_NetInfoReply->deleteLater();
    if(m_NetInfoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        m_NetInfoReply = 0;
        return;
    }
    QByteArray data = m_NetInfoReply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject viewerModel = doc.object()["viewer_model"].toObject();
    QString baseUrl = viewerModel["base_url"].toString();
    QString binUrl = "http://p3d.in" + baseUrl + ".r48.bin";
    qDebug() << "bin url:" << binUrl;
    m_NetDataReply = m_NetMgr->get(QNetworkRequest(QUrl(binUrl)));
    connect(m_NetDataReply, SIGNAL(finished()), SLOT(onModelDataReplyDone()));
    m_NetInfoReply = 0;
}

void QmlAppViewer::onModelDataReplyDone()
{
    m_NetDataReply->deleteLater();
    if(m_NetDataReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        m_NetDataReply = 0;
        return;
    }
    //TODO: threading
    m_extension = ".bin";
    m_ModelData = m_NetDataReply->readAll();
    qDebug() << m_NetDataReply->url() << "returned" << m_ModelData.size() << "bytes";
    setModelState(MS_PROCESSING);
    window->update();
    m_NetDataReply = 0;
}

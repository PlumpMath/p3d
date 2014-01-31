#include "QmlAppViewer.h"
#include <QDebug>
#include <QOpenGLContext>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

#include "P3dViewer.h"
#include "QtPlatformAdapter.h"

QmlAppViewer::QmlAppViewer(QObject *parent) :
    QtQuick2ControlsApplicationViewer(parent)
{
    connect(this, SIGNAL(windowReady()), SLOT(onWindowReady()));
    m_P3dViewer = new P3dViewer(new QtPlatformAdapter());
    m_NetMgr = new QNetworkAccessManager(this);
    m_NetInfoReply = 0;
    m_NetDataReply = 0;

//    loadModel("TpN5G"); // large monkey
//    loadModel("Ui03b"); // horse
//    loadModel("Xczep"); // stone
//    loadModel("eqPpp"); // stone only pos
    loadModel("Pbx7k"); // stone only pos uv
//    loadModel("R7wFq"); // captain
}

QmlAppViewer::~QmlAppViewer()
{
    delete m_P3dViewer;
}

void QmlAppViewer::loadModel(const QString &shortid)
{
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

    m_NetInfoReply = m_NetMgr->get(QNetworkRequest(QUrl("http://p3d.in/api/viewer_models/" + shortid)));
    connect(m_NetInfoReply, SIGNAL(finished()), SLOT(onModelInfoReplyDone()));
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
        //TODO: load the model
        m_P3dViewer->loadModel(m_ModelData.constData(), m_ModelData.size());
        m_ModelData.clear();
    }
    window->resetOpenGLState();
    m_P3dViewer->drawFrame();
}

void QmlAppViewer::onModelInfoReplyDone()
{
    m_NetInfoReply->deleteLater();
    if(m_NetInfoReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
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
}

void QmlAppViewer::onModelDataReplyDone()
{
    m_NetDataReply->deleteLater();
    if(m_NetDataReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        return;
    }
    //TODO: threading
    m_ModelData = m_NetDataReply->readAll();
    qDebug() << m_NetDataReply->url() << "returned" << m_ModelData.size() << "bytes";
    window->update();
}

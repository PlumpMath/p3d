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

static P3dLogger logger("qt.QmlAppViewer", P3dLogger::LOG_DEBUG);

QmlAppViewer::QmlAppViewer(QObject *parent) :
    QtQuick2ControlsApplicationViewer(parent)
{
    qmlRegisterUncreatableType<QmlAppViewer>("p3d.p3dviewer", 1, 0, "Viewer", "Uncreatable");
    engine.rootContext()->setContextProperty("viewer", this);

    connect(this, SIGNAL(windowReady()), SLOT(onWindowReady()));
    m_NetMgr = new QNetworkAccessManager(this);
    m_P3dViewer = new P3dViewer(new QtPlatformAdapter());
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

    m_extension = ".unknown";
    delete m_ModelInfo;
    m_ModelInfo = nullptr;

    setModelState(MS_LOADING);
    if(fileName.endsWith(".blend"))
    {
        m_extension = ".blend";
    }
    else if(fileName.endsWith(".bin"))
    {
        m_extension = ".bin";
    }
    if(m_extension!=".unknown") {
        logger.debug("found a file: %s", path.toUtf8().constData());
        QFile file(path);
        if(!file.exists())
        {
            logger.warning("File doesn't exist: %s", model.toString().toUtf8().constData());
            return;
        }

        file.open(QFile::ReadOnly);
        m_ModelData = file.readAll();
        logger.debug("loaded %d bytes for %s", m_ModelData.size(), m_extension.toUtf8().constData());
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

        if(m_ModelInfo)
        {
            // set diffuse textures
            //TODO: optimize using maps
            QJsonArray mats = m_ModelInfo->value("materials").toArray();
            for(int matIndex = 0, matIndexL = mats.size(); matIndex < matIndexL; ++matIndex)
            {
                QJsonObject mat = mats[matIndex].toObject();
                QJsonArray texAssigns = m_ModelInfo->value("texture_assignments").toArray();
                QJsonArray texs = m_ModelInfo->value("textures").toArray();
                for(QJsonValue texAssignId: mat["texture_assignment_ids"].toArray())
                {
                    for(QJsonValue texAssignValue: texAssigns)
                    {
                        QJsonObject texAssign = texAssignValue.toObject();
                        if(texAssignId.toInt() == texAssign["id"].toInt())
                        {
                            int texId = texAssign["texture_id"].toInt();
                            for(QJsonValue texValue: texs)
                            {
                                QJsonObject tex = texValue.toObject();
                                if(texId == tex["id"].toInt())
                                {
                                    QString texUrl = tex["url"].toString();
                                    QString texType = texAssign["texture_type"].toString();
                                    logger.debug("%d: %s %s", matIndex, texType.toUtf8().constData(), texUrl.toUtf8().constData());
                                    if(texType == "diff")
                                    {
                                        const char* fullUrl = (QString("http://p3d.in") + texUrl).toUtf8().constData();
                                        m_P3dViewer->setMaterialProperty(matIndex, "diffuseTexture", fullUrl);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
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
    m_ModelInfo = new QJsonObject(doc.object());
    QJsonObject viewerModel = (*m_ModelInfo)["viewer_model"].toObject();
    QString baseUrl = viewerModel["base_url"].toString();
    QString binUrl = "http://p3d.in" + baseUrl + ".r48.bin";
    logger.debug("bin url: %s", binUrl.toUtf8().constData());
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
    logger.debug("%s returned %d bytes", m_NetDataReply->url().toString().toUtf8().constData(), m_ModelData.size());
    setModelState(MS_PROCESSING);
    window->update();
    m_NetDataReply = 0;
}

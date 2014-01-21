#include "QmlAppViewer.h"
#include <QDebug>
#include <QOpenGLContext>

#include "P3dViewer.h"

QmlAppViewer::QmlAppViewer(QObject *parent) :
    QtQuick2ControlsApplicationViewer(parent)
{
    connect(this, SIGNAL(windowReady()), SLOT(onWindowReady()));
    m_P3dViewer = new P3dViewer();
}

QmlAppViewer::~QmlAppViewer()
{
    delete m_P3dViewer;
}

void QmlAppViewer::onWindowReady()
{
    connect(window, SIGNAL(beforeRendering()), SLOT(onGLRender()), Qt::DirectConnection);
    connect(window, SIGNAL(sceneGraphInitialized()), SLOT(onGLInit()), Qt::DirectConnection);
}

void QmlAppViewer::onGLInit()
{
    qDebug() << "GL ctx:" << QOpenGLContext::currentContext();
    m_P3dViewer->onSurfaceCreated();
}

void QmlAppViewer::onGLRender()
{
    m_P3dViewer->drawFrame(window->width(), window->height());

    window->resetOpenGLState();
}

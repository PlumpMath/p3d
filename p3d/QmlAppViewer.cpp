#include "QmlAppViewer.h"
#include <QDebug>
#include <QOpenGLContext>

#include "P3dViewer.h"
#include "QtPlatformAdapter.h"

QmlAppViewer::QmlAppViewer(QObject *parent) :
    QtQuick2ControlsApplicationViewer(parent)
{
    connect(this, SIGNAL(windowReady()), SLOT(onWindowReady()));
    m_P3dViewer = new P3dViewer(new QtPlatformAdapter());
}

QmlAppViewer::~QmlAppViewer()
{
    delete m_P3dViewer;
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
    qDebug() << "GL resize;" << window->width() << window->height();
    m_P3dViewer->onSurfaceChanged(window->width(), window->height());
}

void QmlAppViewer::onGLRender()
{
    qDebug() << "GL render";
    m_P3dViewer->drawFrame();

    window->resetOpenGLState();
}

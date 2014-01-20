#include "QmlAppViewer.h"
#include <QDebug>
#include <QOpenGLContext>

#include "P3dViewer.h"

QmlAppViewer::QmlAppViewer(QObject *parent) :
    QtQuick2ControlsApplicationViewer(parent)
{
    connect(this, SIGNAL(windowReady()), SLOT(onWindowReady()));
}

void QmlAppViewer::onWindowReady()
{
    connect(window, SIGNAL(beforeRendering()), SLOT(onGLRender()), Qt::DirectConnection);
    connect(window, SIGNAL(sceneGraphInitialized()), SLOT(onGLInit()), Qt::DirectConnection);
}

void QmlAppViewer::onGLInit()
{
    qDebug() << "GL ctx:" << QOpenGLContext::currentContext();
    on_surface_created();
}

void QmlAppViewer::onGLRender()
{
    on_draw_frame(window->width(), window->height());

    window->resetOpenGLState();
}

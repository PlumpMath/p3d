#ifndef QMLAPPVIEWER_H
#define QMLAPPVIEWER_H

#include "qtquick2controlsapplicationviewer.h"

class P3dViewer;

class QmlAppViewer : public QtQuick2ControlsApplicationViewer
{
    Q_OBJECT
public:
    explicit QmlAppViewer(QObject *parent = 0);
    virtual ~QmlAppViewer();

signals:

public slots:
    void onWindowReady();
    void onGLInit();
    void onGLResize();
    void onGLRender();

private:
    P3dViewer* m_P3dViewer;
};

#endif // QMLAPPVIEWER_H

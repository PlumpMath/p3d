#ifndef QMLAPPVIEWER_H
#define QMLAPPVIEWER_H

#include "qtquick2controlsapplicationviewer.h"

class QmlAppViewer : public QtQuick2ControlsApplicationViewer
{
    Q_OBJECT
public:
    explicit QmlAppViewer(QObject *parent = 0);

signals:

public slots:
    void onWindowReady();
    void onGLInit();
    void onGLRender();
};

#endif // QMLAPPVIEWER_H

#ifndef QMLAPPVIEWER_H
#define QMLAPPVIEWER_H

#include "qtquick2controlsapplicationviewer.h"

class QNetworkAccessManager;
class QNetworkReply;

class P3dViewer;

class QmlAppViewer : public QtQuick2ControlsApplicationViewer
{
    Q_OBJECT
public:
    explicit QmlAppViewer(QObject *parent = 0);
    virtual ~QmlAppViewer();
    Q_INVOKABLE void loadModel(const QString& shortid);

signals:

public slots:
    void onWindowReady();
    void onGLInit();
    void onGLResize();
    void onGLRender();

private slots:
    void onModelInfoReplyDone();
    void onModelDataReplyDone();

private:
    P3dViewer* m_P3dViewer;
    QNetworkAccessManager* m_NetMgr;
    QNetworkReply* m_NetInfoReply;
    QNetworkReply* m_NetDataReply;
    QByteArray m_ModelData;
};

#endif // QMLAPPVIEWER_H

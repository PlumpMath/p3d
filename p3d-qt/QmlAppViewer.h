#ifndef QMLAPPVIEWER_H
#define QMLAPPVIEWER_H

#include "qtquick2controlsapplicationviewer.h"

class QNetworkAccessManager;
class QNetworkReply;

class P3dViewer;
class BlendData;

class QmlAppViewer : public QtQuick2ControlsApplicationViewer
{
    Q_OBJECT
    Q_ENUMS(ModelState)
    Q_PROPERTY(ModelState modelState READ modelState WRITE setModelState NOTIFY modelStateChanged)
public:
    enum ModelState
    {
        MS_NONE,
        MS_LOADING,
        MS_PROCESSING,
        MS_READY
    };

    explicit QmlAppViewer(QObject *parent = 0);
    virtual ~QmlAppViewer();
    ModelState modelState() const {return m_ModelState;}
    void setModelState(ModelState newValue);

    Q_INVOKABLE void loadModel(const QUrl &model);
    Q_INVOKABLE void clearModel();

    Q_INVOKABLE void startRotateCamera(float x, float y);
    Q_INVOKABLE void rotateCamera(float x, float y);
    Q_INVOKABLE void resetCamera();

signals:
    void modelStateChanged();

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
    ModelState m_ModelState;
    bool m_ClearModel;
    QString m_extension;
};

#endif // QMLAPPVIEWER_H

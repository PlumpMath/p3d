#include "QmlAppViewer.h"
#include <QDebug>

int main(int argc, char *argv[])
{

    Application app(argc, argv);
    qSetMessagePattern("%{message}");
    qDebug() << "Starting p3d";

    QmlAppViewer viewer;
    viewer.setMainQmlFile(QStringLiteral("qml/p3d/main.qml"));
    viewer.show();

    return app.exec();
}

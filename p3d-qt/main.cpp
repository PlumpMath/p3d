#include "QmlAppViewer.h"
#include <QDebug>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{

    Application app(argc, argv);
    qSetMessagePattern("%{message}");
    qDebug() << "Starting p3d";

    QCoreApplication::setApplicationVersion("0.1");
    QCommandLineParser parser;
    parser.setApplicationDescription("p3d viewer");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption modelOption(QStringList() << "m" << "model",
                                   "View <model>.",
                                   "model");
    parser.addOption(modelOption);
    // Process the actual command line arguments given by the user
    parser.process(app);
    QString model = parser.value(modelOption);
    qDebug() << "model:" << model;

    QmlAppViewer viewer;
    viewer.setMainQmlFile(QStringLiteral("qml/p3d/main.qml"));
    viewer.show();

    if(!model.isEmpty())
    {
        viewer.window->update();
        app.processEvents();
        viewer.loadModel(model);
    }

//    viewer.loadModel("TpN5G"); // large monkey
//    viewer.loadModel("Ui03b"); // horse
//    viewer.loadModel("Xczep"); // stone
//    viewer.loadModel("eqPpp"); // stone only pos
//    viewer.loadModel("Pbx7k"); // stone only pos uv
//    viewer.loadModel("R7wFq"); // captain
//    viewer.loadModel("zclcJ"); // witch doctor
//    viewer.loadModel("v5JcV");

    return app.exec();
}

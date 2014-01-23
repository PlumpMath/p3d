# Add more folders to ship with the application, here
folder_01.source = qml/p3d
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# The .cpp file which was generated for your project. Feel free to hack it.
INCLUDEPATH += ../libViewer

SOURCES += main.cpp \
    ../libViewer/P3dViewer.cpp \
    QmlAppViewer.cpp \
    ../libViewer/PlatformAdapter.cpp \
    QtPlatformAdapter.cpp \
    ../libViewer/ModelLoader.cpp

# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2controlsapplicationviewer/qtquick2controlsapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    p3d64.png \
    p3d80.png

HEADERS += \
    ../libViewer/glwrapper.h \
    ../libViewer/P3dViewer.h \
    QmlAppViewer.h \
    ../libViewer/PlatformAdapter.h \
    QtPlatformAdapter.h \
    ../libViewer/ModelLoader.h

android {
    shaders.path = /assets/shaders
    shaders.files = $$files($$PWD/../libViewer/shaders/*)
    message($$shaders.files)
    shaders.depends = FORCE
    INSTALLS += shaders
} else {
    shaders.target = shaders
    shaders.commands = ln -s $$PWD/../libViewer/shaders $$OUT_PWD/shaders
    QMAKE_EXTRA_TARGETS += shaders
    PRE_TARGETDEPS += shaders
}

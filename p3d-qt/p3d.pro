TEMPLATE = app

# Add more folders to ship with the application, here
folder_01.source = qml/p3d
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += ../libViewer
INCLUDEPATH += ../ext/glm

SUBDIRS += P3dConverter

# converter paths
INCLUDEPATH += ../libViewer/P3dConverter/P3dConvert
INCLUDEPATH += ../libViewer/P3dConverter/File
INCLUDEPATH += ../libViewer/P3dConverter/FileFormats/Blend

#QMAKE_LFLAGS += -Wl,-rpath,$$absolute_path("../build/p3dconverter/File", $$PWD)
#QMAKE_LFLAGS += -Wl,-rpath,$$absolute_path("../build/p3dconverter/FileFormats/Blend", $$PWD)
#QMAKE_LFLAGS += -Wl,-rpath,$$absolute_path("../build/p3dconverter/P3dConvert", $$PWD)

LIBS += -L./p3dconverter
LIBS += -L./zlib
LIBS += -lzlib -lp3dconverter

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += \
    main.cpp \
    ../libViewer/P3dViewer.cpp \
    QmlAppViewer.cpp \
    ../libViewer/PlatformAdapter.cpp \
    QtPlatformAdapter.cpp \
    ../libViewer/ModelLoader.cpp \
    ../libViewer/CameraNavigation.cpp \
    ../libViewer/BaseLoader.cpp \
    ../libViewer/BinLoader.cpp \
    ../libViewer/BlendLoader.cpp

windows {
SOURCES += \
    ../libViewer/gl3w.c
}

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
    ../libViewer/ModelLoader.h \
    ../libViewer/P3dMap.h \
    ../libViewer/P3dVector.h \
    ../libViewer/CameraNavigation.h \
    ../libViewer/GL/gl3w.h \
    ../libViewer/GL/glcorearb.h \
    ../libViewer/BaseLoader.h \
    ../libViewer/BinLoader.h \
    ../libViewer/BlendLoader.h

RESOURCES += \
    resources.qrc

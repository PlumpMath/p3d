#-------------------------------------------------
#
# Project created by QtCreator 2014-01-20T11:18:33
#
#-------------------------------------------------

TARGET = libViewer
TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH += ../ext/glm

SOURCES += P3dViewer.cpp \
    PlatformAdapter.cpp \
    ModelLoader.cpp \
    CameraNavigation.cpp \
    P3dLogger.cpp

HEADERS += P3dViewer.h \
    glwrapper.h \
    PlatformAdapter.h \
    ModelLoader.h \
    P3dVector.h \
    P3dMap.h \
    CameraNavigation.h \
    P3dLogger.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    shaders/fragment.glsl \
    shaders/vertex.glsl

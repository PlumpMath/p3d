#-------------------------------------------------
#
# Project created by QtCreator 2014-01-20T11:18:33
#
#-------------------------------------------------

TARGET = libViewer
TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH += ../ext/glm

SOURCES += *.cpp

HEADERS += *.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    shaders/fragment.glsl \
    shaders/vertex.glsl

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../libViewer
INCLUDEPATH += ../ext/glm

SOURCES += \
    main.cpp \
    EmPlatformAdapter.cpp

HEADERS += \
    EmPlatformAdapter.h

OTHER_FILES += \
    Makefile \
    shaders/vertex.glsl \
    .gitignore \
    em_test.html \
    library_p3d.js \
    p3d_blend_minimal.html \
    p3d_viewer.js


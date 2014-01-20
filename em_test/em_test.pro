TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp

HEADERS += \
    glwrapper.h

OTHER_FILES += \
    Makefile \
    shaders/fragment.glsl \
    shaders/vertex.glsl \
    .gitignore


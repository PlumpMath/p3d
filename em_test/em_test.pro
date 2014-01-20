TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    game.c \
    main.c

HEADERS += \
    game.h \
    glwrapper.h

OTHER_FILES += \
    Makefile \
    shaders/fragment.glsl \
    shaders/vertex.glsl \
    .gitignore


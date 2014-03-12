INCLUDEPATH += $$PWD/File
INCLUDEPATH += $$PWD/FileFormats/Blend
INCLUDEPATH += $$PWD/FileFormats/Blend/Generated
INCLUDEPATH += $$PWD/P3dConvert

DEFINES += FBT_USE_GZ_FILE=1

SOURCES += \
    $$PWD/File/fbtBuilder.cpp \
    $$PWD/File/fbtFile.cpp \
    $$PWD/File/fbtStreams.cpp \
    $$PWD/File/fbtTables.cpp \
    $$PWD/File/fbtTypes.cpp \
    $$PWD/FileFormats/Blend/fbtBlend.cpp \
    $$PWD/FileFormats/Blend/Generated/bfBlender.cpp \
    $$PWD/P3dConvert/p3dConvert.cpp

HEADERS += \
    $$PWD/File/fbtBuilder.h \
    $$PWD/File/fbtConfig.h \
    $$PWD/File/fbtFile.h \
    $$PWD/File/fbtPlatformHeaders.h \
    $$PWD/File/fbtStreams.h \
    $$PWD/File/fbtTables.h \
    $$PWD/File/fbtTypes.h \
    $$PWD/FileFormats/Blend/Blender.h \
    $$PWD/FileFormats/Blend/fbtBlend.h \
    $$PWD/P3dConvert/p3dConvert.h

include($$PWD/zlib/zlib.pri)


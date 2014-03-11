TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += File
INCLUDEPATH += FileFormats/Blend
INCLUDEPATH += FileFormats/Blend/Generated
INCLUDEPATH += P3dConvert
INCLUDEPATH += zlib

SUBDIRS += zlib

SOURCES += \
	File/fbtBuilder.cpp \
	File/fbtFile.cpp \
	File/fbtStreams.cpp \
	File/fbtTables.cpp \
	File/fbtTypes.cpp \
	FileFormats/Blend/fbtBlend.cpp \
	FileFormats/Blend/Generated/bfBlender.cpp \
	P3dConvert/p3dConvert.cpp

HEADERS += \
	File/fbtBuilder.h \
	File/fbtConfig.h \
	File/fbtFile.h \
	File/fbtPlatformHeaders.h \
	File/fbtStreams.h \
	File/fbtTables.h \
	File/fbtTypes.h \
	FileFormats/Blend/Blender.h \
	FileFormats/Blend/fbtBlend.h \
	P3dConvert/p3dConvert.h


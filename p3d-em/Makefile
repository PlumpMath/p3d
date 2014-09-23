OPTIMIZE = -Oz
LDOPTIMIZE = -O3 --closure 1
#OPTIMIZE = -O0
#LDOPTIMIZE = -O0

EXPORTED_FUNCTIONS = "[\
'_main',\
'_loadModel',\
'_startRotateCam',\
'_rotateCam',\
'_resetCam',\
'_zoomCam',\
'_materialCount',\
'_setMaterialProperty'\
]"

INCLUDE_DIRS = \
    -I../libViewer\
    -I../ext/glm\
    -I../libViewer/P3dConverter/File\
    -I../libViewer/P3dConverter/FileFormats\
    -I../libViewer/P3dConverter/FileFormats/Blend\
    -I../libViewer/P3dConverter/P3dConvert

WARNINGS = -Wno-c++11-narrowing

CFLAGS = -I. $(INCLUDE_DIRS) -Wall -Wextra $(WARNINGS) $(OPTIMIZE)
CXXFLAGS = -I. $(INCLUDE_DIRS) -Wall -Wextra $(WARNINGS) -std=c++0x $(OPTIMIZE)
#LDFLAGS = -s ASM_JS=1 -s EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS)  --embed-file shaders $(OPTIMIZE) $(LDOPTIMIZE)
LDFLAGS = -s ASM_JS=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS) --preload-file shaders --memory-init-file 0 \
    $(LDOPTIMIZE) --js-library library_p3d.js

LIBVIEWER_DIR = ../libViewer
LIBVIEWER_SOURCES = \
    P3dViewer.cpp \
    PlatformAdapter.cpp \
    P3dLogger.cpp \
    ModelLoader.cpp \
    BaseLoader.cpp \
    BinLoader.cpp \
    BlendLoader.cpp \
    CameraNavigation.cpp

ZLIB_DIR = ../libViewer/P3dConverter/zlib
ZLIB_SOURCES = \
    adler32.c \
    compress.c \
    crc32.c \
    deflate.c \
    gzclose.c \
    gzlib.c \
    gzread.c \
    gzwrite.c \
    infback.c \
    inffast.c \
    inflate.c \
    inftrees.c \
    trees.c \
    uncompr.c \
    zutil.c

P3DCONVERTER_DIR = ../libViewer/P3dConverter
P3DCONVERTER_SOURCES = \
    fbtBuilder.cpp \
    fbtFile.cpp \
    fbtStreams.cpp \
    fbtTables.cpp \
    fbtTypes.cpp \
    fbtBlend.cpp \
    bfBlender.cpp \
    p3dConvert.cpp


SOURCES = \
    main.cpp \
    EmPlatformAdapter.cpp \
    $(LIBVIEWER_SOURCES) $(ZLIB_SOURCES) $(P3DCONVERTER_SOURCES)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES:.cpp=.o))
VPATH = \
    ../ext/glm \
    $(LIBVIEWER_DIR) \
    $(ZLIB_DIR) \
    $(P3DCONVERTER_DIR)/File \
    $(P3DCONVERTER_DIR)/FileFormats \
    $(P3DCONVERTER_DIR)/FileFormats/Blend \
    $(P3DCONVERTER_DIR)/FileFormats/Blend/Generated \
    $(P3DCONVERTER_DIR)/P3dConvert

TARGET = p3d_engine.js
TARGET_GZ = $(TARGET).gz
SHADERS = $(shell find $(LIBVIEWER_DIR)/shaders/ -name "*.glsl")

# Targets start here.
all: $(TARGET_GZ)

$(CURDIR)/shaders: $(LIBVIEWER_DIR)/shaders
	ln -s $< $@

$(TARGET): $(OBJECTS) Makefile deps.txt $(CURDIR)/shaders $(SHADERS) library_p3d.js
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJECTS)

$(TARGET_GZ): $(TARGET)
	gzip -c $< > $@

clean:
	$(RM) $(TARGET) $(TARGET_GZ) $(OBJECTS) deps.txt $(TARGET).mem $(basename $(TARGET)).data

deps.txt: $(SOURCES) Makefile
	@$(CC) $(CFLAGS) -MM  $^ > $@



# list targets that do not create files (but not all makes understand .PHONY)
.PHONY:	all clean

$(OBJECTS): Makefile

-include deps.txt


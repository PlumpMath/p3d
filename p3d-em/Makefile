OPTIMIZE = -O2
LDOPTIMIZE = --closure 1

CFLAGS = -I. -I../libViewer -I../ext/glm -Wall -Wextra $(OPTIMIZE)
CXXFLAGS = -I. -I../libViewer -I../ext/glm -Wall -Wextra -std=c++0x $(OPTIMIZE)
#LDFLAGS = -s ASM_JS=1 -s EXPORTED_FUNCTIONS="['_main', '_loadBinary']"  --embed-file shaders $(OPTIMIZE) $(LDOPTIMIZE)
LDFLAGS = -s ASM_JS=1 -s EXPORTED_FUNCTIONS="['_main', '_loadBinary']"  --preload-file shaders --memory-init-file 1 $(OPTIMIZE) $(LDOPTIMIZE)

LIBVIEWER_DIR = ../libViewer
LIBVIEWER_SOURCES = P3dViewer.cpp PlatformAdapter.cpp ModelLoader.cpp
SOURCES = main.cpp $(LIBVIEWER_SOURCES)
OBJECTS = $(SOURCES:.cpp=.o)
VPATH = $(LIBVIEWER_DIR) ../ext/glm
TARGET = em_test.js
SHADERS = $(shell find $(LIBVIEWER_DIR)/shaders/ -name "*.glsl")

# Targets start here.
all: $(TARGET)

shaders: $(LIBVIEWER_DIR)/shaders
	ln -s $< $@

$(TARGET): $(OBJECTS) Makefile deps.txt shaders $(SHADERS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJECTS)

clean:
	$(RM) $(TARGET) $(OBJECTS) deps.txt $(TARGET).mem $(basename $(TARGET)).data

deps.txt: $(SOURCES) Makefile
	@$(CC) $(CFLAGS) -MM  $^ > $@

# list targets that do not create files (but not all makes understand .PHONY)
.PHONY:	all clean

$(OBJECTS): Makefile

-include deps.txt


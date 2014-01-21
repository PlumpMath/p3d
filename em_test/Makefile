OPTIMIZE = -O3
CFLAGS = -I. -I../libViewer -Wall -Wextra $(OPTIMIZE)
CXXFLAGS = -I. -I../libViewer -Wall -Wextra $(OPTIMIZE)
LDFLAGS = -s ASM_JS=1 --embed-file shaders $(OPTIMIZE)
#LDFLAGS = -s ASM_JS=1 --preload-file shaders --memory-init-file 1

SOURCES = main.cpp ../libViewer/P3dViewer.cpp ../libViewer/PlatformAdapter.cpp
OBJECTS = main.o P3dViewer.o PlatformAdapter.o
TARGET = em_test.html
SHADERS = $(shell find ../libViewer/shaders/ -name "*.glsl")

# Targets start here.
all: $(TARGET)

shaders: ../libViewer/shaders
	ln -s $< $@

P3dViewer.o: ../libViewer/P3dViewer.cpp
	$(CC) -c $(CFLAGS) $< -o $@

PlatformAdapter.o: ../libViewer/PlatformAdapter.cpp
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJECTS) Makefile shaders $(SHADERS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJECTS)

clean:
	$(RM) $(TARGET) $(OBJECTS) deps.txt $(TARGET).mem $(basename $(TARGET)).data

deps.txt: $(SOURCES)
	@$(CC) $(CFLAGS) -MM  $(SOURCES) > $@
	
# list targets that do not create files (but not all makes understand .PHONY)
.PHONY:	all clean

$(OBJECTS): Makefile deps.txt

-include deps.txt


CFLAGS = -I. -Wall -Wextra -O3
LDFLAGS = -s ASM_JS=1 --embed-file shaders

SOURCES = main.c game.c
OBJECTS = main.o game.o
TARGET = em_test.html
SHADERS = $(shell find shaders/ -name "*.glsl")

# Targets start here.
all: $(TARGET)

$(TARGET): $(OBJECTS) Makefile $(SHADERS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJECTS)

clean:
	$(RM) $(TARGET) $(OBJECTS) deps.txt

deps.txt: $(SOURCES)
	@$(CC) $(CFLAGS) -MM  $(SOURCES) > $@
	
# list targets that do not create files (but not all makes understand .PHONY)
.PHONY:	all clean

$(OBJECTS): Makefile deps.txt

-include deps.txt


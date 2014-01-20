#ifdef EMSCRIPTEN
#include <GLES2/gl2.h>
#endif
#ifdef QT_GUI_LIB
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

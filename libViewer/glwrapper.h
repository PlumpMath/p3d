#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
#endif

#ifdef QT_GUI_LIB
#include <qglobal.h>
#define GL_GLEXT_PROTOTYPES

#ifdef QT_OPENGL_ES_2
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#endif

#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
#endif //__EMSCRIPTEN__

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#endif //__ANDROID__

#ifdef QT_GUI_LIB
#include <qglobal.h>
#define GL_GLEXT_PROTOTYPES

#ifdef QT_OPENGL_ES_2
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif //QT_OPENGL_ES_2

#endif //QT_GUI_LIB


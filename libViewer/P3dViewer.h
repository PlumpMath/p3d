#ifndef P3DVIEWER_H
#define P3DVIEWER_H

#include <cstdlib>

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;

class PlatformAdapter;
class ModelLoader;
class CameraNavigation;

const GLuint ATTRIB_POSITION = 0;
const GLuint ATTRIB_NORMAL = 1;
const GLuint ATTRIB_UV = 2;

class P3dViewer
{
public:
    //! \brief creates a P3dViewer
    //! \arg adapter PlatformAdaper
    //! \note P3dViewer takes ownership of adapter and will delete it in dtor
    P3dViewer(PlatformAdapter* adapter = 0);
    virtual ~P3dViewer();
    void clear();
    void onSurfaceCreated();
    void onSurfaceChanged(int width, int height);
    void drawFrame();
    bool loadModel(const char* binaryData, size_t size);
    CameraNavigation* cameraNavigation() {return m_CameraNavigation;}

private:
    GLuint loadShader(GLenum type, const char *shaderSrc, size_t shaderSize, const char *shaderName);
    GLuint loadShaderFromFile(GLenum type, const char *shaderFile, const char *defines = 0);
    GLuint loadProgram(const char* vShaderFile, const char* fShaderFile, const char *defines = 0);

    ModelLoader* m_ModelLoader;
    CameraNavigation* m_CameraNavigation;
    GLuint m_ProgramObject;
    GLuint m_ProgramObjectUv;

    GLint m_UniformMVP;
    GLint m_UniformMVPUv;

    int m_Width;
    int m_Height;
    bool m_InitOk;
};

#endif // P3DVIEWER_H

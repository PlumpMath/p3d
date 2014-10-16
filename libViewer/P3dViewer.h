#ifndef P3DVIEWER_H
#define P3DVIEWER_H

#include <cstdlib>
#include "P3dVector.h"
#include "IMaterialsInfo.h"

#define GLM_FORCE_RADIANS
// vec3, vec4, ivec4, mat4
#include <glm/glm.hpp>

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;

class PlatformAdapter;
class ModelLoader;
class CameraNavigation;

class BlendData;

const GLuint ATTRIB_POSITION = 0;
const GLuint ATTRIB_NORMAL = 1;
const GLuint ATTRIB_UV = 2;

class P3dViewer: public IMaterialsInfo
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
    bool loadModel(const char* binaryData, size_t size, const char* extension);
    void clearModel();
    CameraNavigation* cameraNavigation() {return m_CameraNavigation;}

    int materialCount();
    void setMaterialProperty(int materialIndex, const char* property, const char* value);

    void parseColor(glm::vec3 &color, const char *value);
private:
    struct P3dMaterial
    {
        // diffuse
        GLuint diffuseTexture = 0;
        glm::vec3 diff_col{1.0, 1.0, 1.0};
        float diff_str = 0.5f;
        float diff_tex_str = 1.0f;

        // specular
        GLuint specTexture = 0;
        glm::vec3 spec_col{1.0, 1.0, 1.0};
        float spec_shininess = 0.1f;
        float spec_str = 0.15f;
    };

    GLuint loadShader(GLenum type, const char *shaderSrc, size_t shaderSize, const char *shaderName);
    GLuint loadShaderFromFile(GLenum type, const char *shaderFile, const char *defines = 0);
    GLuint loadProgram(const char* vShaderFile, const char* fShaderFile, const char *defines = 0);
    GLint getUniform(GLuint program, const char* name);

    ModelLoader* m_ModelLoader;
    CameraNavigation* m_CameraNavigation;

    enum programs
    {
        BASIC = 0,
        UVS = 1
    };

    static const int programCount = 2;
    GLuint m_Programs[programCount] = {0, 0};

    P3dVector<P3dMaterial> m_Materials;

    int m_Width;
    int m_Height;
    bool m_InitOk = false;
};

#endif // P3DVIEWER_H

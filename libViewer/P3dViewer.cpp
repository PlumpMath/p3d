#include "P3dViewer.h"
#include "PlatformAdapter.h"
#include "ModelLoader.h"
#include "CameraNavigation.h"
#include "glwrapper.h"

#define GLM_FORCE_RADIANS
// vec3, vec4, ivec4, mat4
#include <glm/glm.hpp>
// translate, rotate, scale, perspective
#include <glm/gtc/matrix_transform.hpp>
// value_ptr
#include <glm/gtc/type_ptr.hpp>

const float PI = 3.14159265358979f;
const float D2R = PI / 180;

P3dViewer::P3dViewer(PlatformAdapter* adapter)
{
    PlatformAdapter::adapter = adapter;
    if(!PlatformAdapter::adapter)
    {
        PlatformAdapter::adapter = new PlatformAdapter();
    }

    m_ProgramObject = 0;
    m_ProgramObjectUv = 0;
    m_InitOk = false;
    m_ModelLoader = new ModelLoader();
    m_CameraNavigation = new CameraNavigation();

    P3D_LOGD("Viewer constructed");
}

P3dViewer::~P3dViewer()
{
    delete m_CameraNavigation;
    delete m_ModelLoader;

    delete PlatformAdapter::adapter;
}

void P3dViewer::clear()
{
    // free shaders
    if(m_ProgramObject) glDeleteProgram(m_ProgramObject);
    if(m_ProgramObjectUv) glDeleteProgram(m_ProgramObjectUv);
    m_InitOk = false;
}

GLuint P3dViewer::loadShader (GLenum type, const char *shaderSrc, size_t shaderSize, const char* shaderName)
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    if ( shader == 0 )
        return 0;

    GLint srcLen = shaderSize;

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, &srcLen);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv (shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char* infoLog = new char[sizeof(char) * infoLen ];

            glGetShaderInfoLog(shader, infoLen, 0, infoLog);
            P3D_LOGE( "Error compiling shader %s:\n%s", shaderName, infoLog);

            delete[] infoLog;
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint P3dViewer::loadShaderFromFile (GLenum type, const char *shaderFile, const char* defines)
{
    size_t srcLen = 0;
    GLuint shader = 0;
    const char* shaderSrc = PlatformAdapter::adapter->loadAsset(shaderFile, &srcLen);
    if(defines)
    {
        size_t defsLen = strlen(defines);
        char* tmp = new char[srcLen + defsLen];
        strncpy(tmp, shaderSrc, srcLen);
        strncpy(tmp + srcLen, defines, defsLen);
        shader = loadShader(type, tmp, srcLen + defsLen, shaderFile);
        delete[] tmp;
    }
    else
    {
        shader = loadShader(type, shaderSrc, srcLen, shaderFile);
    }
    delete[] shaderSrc;
    return shader;
}

GLuint P3dViewer::loadProgram(const char *vShaderFile, const char *fShaderFile, const char *defines)
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = loadShaderFromFile(GL_VERTEX_SHADER, vShaderFile, defines);
    fragmentShader = loadShaderFromFile(GL_FRAGMENT_SHADER, fShaderFile, defines);

    // Create the program object
    program = glCreateProgram();

    if(program == 0)
    {
        P3D_LOGE("Could create shader program");
        return 0;
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Bind attribute indices
    glBindAttribLocation(program, ATTRIB_POSITION, "aPosition");
    glBindAttribLocation(program, ATTRIB_NORMAL, "aNormal");
    glBindAttribLocation(program, ATTRIB_UV, "aUv");

    // Link the program
    glLinkProgram(program);

    // Check the link status
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if(!linked)
    {
        GLint infoLen = 0;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char* infoLog = new char[sizeof(char) * infoLen];

            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            P3D_LOGE( "Error linking program:\n%s", infoLog );

            delete[] infoLog;
        }

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

void P3dViewer::onSurfaceCreated() {
    m_InitOk = false;

    m_ProgramObject = loadProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    m_UniformMVP = glGetUniformLocation(m_ProgramObject, "uMVP");
    if (m_UniformMVP == -1) {
      P3D_LOGE("Could not bind uniform %s", "uMVP");
      glDeleteProgram(m_ProgramObject);
    }

    m_ProgramObjectUv = loadProgram("shaders/vertex.glsl", "shaders/fragment.glsl", "#define HAS_UV\n");
    m_UniformMVPUv = glGetUniformLocation(m_ProgramObjectUv, "uMVP");
    if (m_UniformMVPUv == -1) {
      P3D_LOGE("Could not bind uniform %s", "uMVP");
      glDeleteProgram(m_ProgramObjectUv);
    }

    int depth;
    glGetIntegerv(GL_DEPTH_BITS, &depth);
    P3D_LOGD("Depth buffer: %d bits", depth);
    m_InitOk = true;
}

void P3dViewer::onSurfaceChanged(int width, int height) {
	P3D_LOGD("resize %d, %d", width, height);
    m_Width = width;
    m_Height = height;
}

void P3dViewer::drawFrame() {
    if(!m_InitOk) {
        return;
    }

    // Set the viewport
    glViewport(0, 0, m_Width, m_Height);

    // Clear color, depth, stencil buffers
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);

    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW);

    if(m_ModelLoader->isLoaded() && m_ModelLoader->boundingRadius() > 0.0f)
    {
        // MVP
        float near = m_ModelLoader->boundingRadius() * 2.2f;
        float far = m_ModelLoader->boundingRadius() * 3.8f;
        glm::mat4 view = m_CameraNavigation->viewMatrix();
        glm::mat4 proj = glm::perspective(25.0f * D2R, 1.0f * m_Width / m_Height, near, far);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP = proj * view * model;

        glBindBuffer(GL_ARRAY_BUFFER, m_ModelLoader->posBuffer());
        glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(ATTRIB_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, m_ModelLoader->normBuffer());
        glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(ATTRIB_NORMAL);

        glBindBuffer(GL_ARRAY_BUFFER, m_ModelLoader->uvBuffer());
        glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(ATTRIB_UV);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ModelLoader->indexBuffer());

        if(m_ModelLoader->indexCount(ModelLoader::VT_POS_UV_NORM))
        {
            // pos uv norm
            glUseProgram(m_ProgramObjectUv);
            glUniformMatrix4fv(m_UniformMVPUv, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawElements(GL_TRIANGLES, m_ModelLoader->indexCount(ModelLoader::VT_POS_UV_NORM),
                           GL_UNSIGNED_INT, (GLvoid*)(size_t)m_ModelLoader->indexOffset(ModelLoader::VT_POS_UV_NORM));
        }
        if(m_ModelLoader->indexCount(ModelLoader::VT_POS_UV))
        {
            // pos uv norm
            glUseProgram(m_ProgramObjectUv);
            glUniformMatrix4fv(m_UniformMVPUv, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawElements(GL_TRIANGLES, m_ModelLoader->indexCount(ModelLoader::VT_POS_UV),
                           GL_UNSIGNED_INT, (GLvoid*)(size_t)m_ModelLoader->indexOffset(ModelLoader::VT_POS_UV));
        }
        if(m_ModelLoader->indexCount(ModelLoader::VT_POS_NORM))
        {
            // pos norm
            glUseProgram(m_ProgramObject);
            glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawElements(GL_TRIANGLES, m_ModelLoader->indexCount(ModelLoader::VT_POS_NORM),
                           GL_UNSIGNED_INT, (GLvoid*)(size_t)m_ModelLoader->indexOffset(ModelLoader::VT_POS_NORM));
        }
        if(m_ModelLoader->indexCount(ModelLoader::VT_POS))
        {
            // pos norm
            glUseProgram(m_ProgramObject);
            glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawElements(GL_TRIANGLES, m_ModelLoader->indexCount(ModelLoader::VT_POS),
                           GL_UNSIGNED_INT, (GLvoid*)(size_t)m_ModelLoader->indexOffset(ModelLoader::VT_POS));
        }
    }
}

bool P3dViewer::loadModel(const char *binaryData, size_t size)
{
    bool res = m_ModelLoader->load(binaryData, size);
    if(res)
    {
        m_CameraNavigation->setBoundingRadius(m_ModelLoader->boundingRadius());
        m_CameraNavigation->reset();
    }
    return res;
}


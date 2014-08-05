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

static P3dLogger logger("core.P3dViewer", P3dLogger::LOG_DEBUG);

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

    logger.debug("Viewer constructed");
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
            logger.error("Error compiling shader %s:\n%s", shaderName, infoLog);

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
        strncpy(tmp, defines, defsLen);
        strncpy(tmp + defsLen, shaderSrc, srcLen);
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
        logger.error("Could create shader program");
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
            logger.error( "Error linking program:\n%s", infoLog );

            delete[] infoLog;
        }

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

void P3dViewer::onSurfaceCreated() {
    m_InitOk = false;
#ifdef __gl3w_h_
    if(gl3wInit())
    {
        logger.error("Unable to initialize OpenGL");
        return;
    }
#endif

    m_ProgramObject = loadProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    m_UniformMVP = glGetUniformLocation(m_ProgramObject, "uMVP");
    if (m_UniformMVP == -1) {
      logger.error("Could not bind uniform %s", "uMVP");
      glDeleteProgram(m_ProgramObject);
    }

    m_ProgramObjectUv = loadProgram("shaders/vertex.glsl", "shaders/fragment.glsl", "#define HAS_UV\n");
    m_UniformMVPUv = glGetUniformLocation(m_ProgramObjectUv, "uMVP");
    if (m_UniformMVPUv == -1) {
      logger.error("Could not bind uniform %s", "uMVP");
      glDeleteProgram(m_ProgramObjectUv);
    }

    int depth;
    glGetIntegerv(GL_DEPTH_BITS, &depth);
    logger.debug("Depth buffer: %d bits", depth);
    m_InitOk = true;
}

void P3dViewer::onSurfaceChanged(int width, int height) {
    logger.debug("resize %d, %d", width, height);
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

    if(m_ModelLoader->isLoaded() && m_ModelLoader->boundingRadius() > 0.0f)
    {

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(true);

        glDisable(GL_CULL_FACE);
        //glEnable(GL_CULL_FACE);
        //glFrontFace(GL_CCW);

        glDisable(GL_SCISSOR_TEST);

        // MVP
        float nearPlane = m_ModelLoader->boundingRadius() * 2.2f;
        float farPlane = m_ModelLoader->boundingRadius() * 3.8f;
        glm::mat4 view = m_CameraNavigation->viewMatrix();
        glm::mat4 proj = glm::perspective(25.0f * D2R, 1.0f * m_Width / m_Height, nearPlane, farPlane);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP = proj * view * model;

        for(int chunk = 0, chunkl = m_ModelLoader->chunkCount(); chunk < chunkl; ++chunk)
        {
            if(m_ModelLoader->indexCount(chunk))
            {
                GLuint arrayBuffer;

                arrayBuffer = m_ModelLoader->posBuffer(chunk);
                glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
                glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(ATTRIB_POSITION);

                arrayBuffer = m_ModelLoader->normBuffer(chunk);
                glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
                if(arrayBuffer)
                {
                    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
                    glEnableVertexAttribArray(ATTRIB_NORMAL);
                }
                else
                {
                    glDisableVertexAttribArray(ATTRIB_NORMAL);
                }

                arrayBuffer = m_ModelLoader->uvBuffer(chunk);
                glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
                if(arrayBuffer)
                {
                    glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, 0, 0);
                    glEnableVertexAttribArray(ATTRIB_UV);
                }
                else
                {
                    glDisableVertexAttribArray(ATTRIB_UV);
                }

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ModelLoader->indexBuffer());

                GLuint uDiffuse = 0;
                if(m_ModelLoader->hasUvs(chunk))
                {
                    // has uvs
                    glUseProgram(m_ProgramObjectUv);
                    glUniformMatrix4fv(m_UniformMVPUv, 1, GL_FALSE, glm::value_ptr(MVP));

                    uDiffuse = glGetUniformLocation(m_ProgramObjectUv, "uDiffuse");
                }
                else
                {
                    // no uvs
                    glUseProgram(m_ProgramObject);
                    glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, glm::value_ptr(MVP));

                    uDiffuse = glGetUniformLocation(m_ProgramObject, "uDiffuse");
                }

                static glm::vec3 colors[] = {
                    glm::vec3(1.0f, 1.0f, 1.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                };

                glm::vec3& color = colors[m_ModelLoader->material(chunk) % (sizeof(colors) / sizeof(colors[0]))];
                glUniform3f(uDiffuse, color.r, color.g, color.b);

                GLsizei count = m_ModelLoader->indexCount(chunk);
                uint32_t offset = m_ModelLoader->indexOffset(chunk);
                glDrawElements(GL_TRIANGLES, count,
                               GL_UNSIGNED_SHORT,
                               (GLvoid*)(sizeof(GLushort) * offset));
                }
        }
    }
}

bool P3dViewer::loadModel(const char *binaryData, size_t size, const char *extension)
{
    BaseLoader* loader = BaseLoader::loaderFromExtension(extension);
    if(!loader)
    {
        logger.warning("unsupported extension:  %s", extension);
        return false;
    }
    loader->setModelLoader(m_ModelLoader);
    bool res = loader->load(binaryData, size);
    if(res)
    {
        logger.debug("bounding radius %f", m_ModelLoader->boundingRadius());
        m_CameraNavigation->setBoundingRadius(m_ModelLoader->boundingRadius());
        m_CameraNavigation->reset();
    }
    return res;
}

void P3dViewer::clearModel()
{
    m_ModelLoader->clear();
}

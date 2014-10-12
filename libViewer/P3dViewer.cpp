#include "P3dViewer.h"
#include "PlatformAdapter.h"
#include "ModelLoader.h"
#include "CameraNavigation.h"
#include "glwrapper.h"

// translate, rotate, scale, perspective
#include <glm/gtc/matrix_transform.hpp>
// value_ptr
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>

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

GLint P3dViewer::getUniform(GLuint program, const char *name)
{
    GLint res = glGetUniformLocation(program, name);
    if (res == -1) {
      logger.error("Could not bind uniform %s", name);
    }
    return res;
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

    m_ProgramObject = loadProgram("shaders/vertex.glsl", "shaders/fragment.glsl",
                                  "#define MAX_DIR_LIGHTS 4\n");
    m_UniformMVP = getUniform(m_ProgramObject, "uMVP");
    m_UniformViewMatrix = getUniform(m_ProgramObject, "viewMatrix");

    m_ProgramObjectUv = loadProgram("shaders/vertex.glsl", "shaders/fragment.glsl",
                                    "#define MAX_DIR_LIGHTS 4\n"
                                    "#define HAS_UV\n"
                                    "#define USE_DIFFUSE_TEXTURE\n");
    m_UniformMVPUv = getUniform(m_ProgramObjectUv, "uMVP");
    m_UniformViewMatrixUv = getUniform(m_ProgramObjectUv, "viewMatrix");
    m_UniformTDiffuse = getUniform(m_ProgramObjectUv, "tDiffuse");
    m_UniformEnableDiffuse = getUniform(m_ProgramObjectUv, "enableDiffuse");

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
        float camDist = m_CameraNavigation->cameraDist();
        float nearPlane = camDist - m_ModelLoader->boundingRadius() * 1.1f;
        if(nearPlane <= 0) nearPlane = 0.01f;
        float farPlane = camDist + m_ModelLoader->boundingRadius() * 2.2f;
        glm::mat4 view = m_CameraNavigation->viewMatrix();
        glm::mat4 proj = glm::perspective(25.0f * D2R, 1.0f * m_Width / m_Height, nearPlane, farPlane);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 modelView = view * model;
        glm::mat4 MVP = proj * modelView;
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelView)));

        for(int chunk = 0, chunkl = m_ModelLoader->chunkCount(); chunk < chunkl; ++chunk)
        {
            if(m_ModelLoader->indexCount(chunk))
            {
                GLuint arrayBuffer;
                P3dMaterial& material = m_Materials[m_ModelLoader->material(chunk)];

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

                GLuint programObject = 0;
                GLuint uDiffuseColor = 0;
                if(m_ModelLoader->hasUvs(chunk))
                {
                    // has uvs
                    programObject = m_ProgramObjectUv;
                    glUseProgram(m_ProgramObjectUv);
                    glUniformMatrix4fv(m_UniformMVPUv, 1, GL_FALSE, glm::value_ptr(MVP));
                    glUniformMatrix4fv(m_UniformViewMatrixUv, 1, GL_FALSE, glm::value_ptr(view));

                    // texure
                    GLuint diffuseTexId = material.diffuseTexture;
                    glUniform1i(m_UniformEnableDiffuse, diffuseTexId != 0);
                    if(diffuseTexId)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, diffuseTexId);
                        glUniform1i(m_UniformTDiffuse, 0);
                    }
                }
                else
                {
                    // no uvs
                    programObject = m_ProgramObject;
                    glUseProgram(m_ProgramObject);
                    glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, glm::value_ptr(MVP));
                    glUniformMatrix4fv(m_UniformViewMatrix, 1, GL_FALSE, glm::value_ptr(view));
                }
                uDiffuseColor = glGetUniformLocation(programObject, "uDiffuseColor");
                glUniform3fv(uDiffuseColor, 1, glm::value_ptr(material.diff_col));

                GLuint uNormalMatrix = glGetUniformLocation(programObject, "normalMatrix");
                glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

                // lights
                GLuint directionalLightColor = 0;
                GLuint directionalLightDirection = 0;
                directionalLightColor = glGetUniformLocation(programObject, "directionalLightColor");
                directionalLightDirection = glGetUniformLocation(programObject, "directionalLightDirection");
                glm::vec3 lightColors[4] = {
                    glm::vec3(0xff, 0xfa, 0xf0) * (1.15f / 255.0f),
                    glm::vec3(0xb3, 0xe5, 0xff) * (0.55f / 255.0f),
                    glm::vec3(0xfd, 0xff, 0xcc) * (0.55f / 255.0f),
                    glm::vec3(0xb3, 0xe5, 0xff) * (0.55f / 255.0f)
                };
                glm::vec3 lightDirs[4] = {
                    glm::vec3(10, 10, 10) * normalMatrix,
                    glm::vec3(-5, 10, 5) * normalMatrix,
                    glm::vec3(0, -10, 5) * normalMatrix,
                    glm::vec3(0, 0, -10) * normalMatrix
                };

                glUniform3fv(directionalLightColor, 4, reinterpret_cast<GLfloat*>(lightColors));
                glUniform3fv(directionalLightDirection, 4, reinterpret_cast<GLfloat*>(lightDirs));

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
    clearModel();
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
        logger.debug("material count: %d", materialCount());
        for(int i = 0, il = materialCount(); i < il; ++i)
        {
            m_Materials.push_back(P3dMaterial());
        }
        logger.debug("bounding radius %f", m_ModelLoader->boundingRadius());
        m_CameraNavigation->setBoundingRadius(m_ModelLoader->boundingRadius());
        m_CameraNavigation->reset();
    }
    return res;
}

void P3dViewer::clearModel()
{
    m_ModelLoader->clear();

    PlatformAdapter::adapter->cancelTextureLoads();
    for(P3dMaterial material: m_Materials)
    {
        PlatformAdapter::adapter->deleteTexture(material.diffuseTexture);
    }
    m_Materials.clear();
}

int P3dViewer::materialCount()
{
    return m_ModelLoader->materialCount();
}

void P3dViewer::setMaterialProperty(int materialIndex, const char *property, const char *value)
{
    if(materialIndex >= materialCount())
    {
        return;
    }

    P3dMaterial& material = m_Materials[materialIndex];

    logger.verbose("setMaterialProperty %d: %s = %s", materialIndex, property, value);

    if(!strcmp("diffuseTexture", property))
    {
        PlatformAdapter::adapter->loadTexture(value, [=,&material](uint32_t texId)
        {
            material.diffuseTexture = texId;
        });
    }

    if(!strcmp("diff_col", property))
    {
        uint32_t uval = strtoul(value, nullptr, 16);
        material.diff_col.r = ((uval & 0xff0000) >> 16) / 255.0f;
        material.diff_col.g = ((uval & 0xff00) >> 8) / 255.0f;
        material.diff_col.b = (uval & 0xff) / 255.0f;
    }
}

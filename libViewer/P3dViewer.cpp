#include "P3dViewer.h"
#include "PlatformAdapter.h"
#include "ModelLoader.h"
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
    m_VertexPosObject = 0;
    m_InitOk = false;
    m_ModelLoader = new ModelLoader();

    P3D_LOGD("Viewer constructed");
}

P3dViewer::~P3dViewer()
{
    delete m_ModelLoader;

    delete PlatformAdapter::adapter;
}

GLuint P3dViewer::loadShader (GLenum type, const char *shaderSrc, const char* shaderName)
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader ( type );

    if ( shader == 0 )
        return 0;

    // Load the shader source
    glShaderSource ( shader, 1, &shaderSrc, 0 );

    // Compile the shader
    glCompileShader ( shader );

    // Check the compile status
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

    if ( !compiled )
    {
        GLint infoLen = 0;

        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = new char[sizeof(char) * infoLen ];

            glGetShaderInfoLog ( shader, infoLen, 0, infoLog );
            P3D_LOGE( "Error compiling shader %s:\n%s", shaderName, infoLog );

            delete[] infoLog;
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;
}

GLuint P3dViewer::loadShaderFromFile (GLenum type, const char *shaderFile)
{
    const char* shaderSrc = PlatformAdapter::adapter->loadAsset(shaderFile);
    GLuint shader = loadShader(type, shaderSrc, shaderFile);
    delete[] shaderSrc;
    return shader;
}

void P3dViewer::onSurfaceCreated() {

    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = loadShaderFromFile ( GL_VERTEX_SHADER, "shaders/vertex.glsl" );
    fragmentShader = loadShaderFromFile ( GL_FRAGMENT_SHADER, "shaders/fragment.glsl" );

    // Create the program object
    m_ProgramObject = glCreateProgram ( );

    if ( m_ProgramObject == 0 )
       return;

    glAttachShader ( m_ProgramObject, vertexShader );
    glAttachShader ( m_ProgramObject, fragmentShader );

    // Bind attribute indices
    glBindAttribLocation( m_ProgramObject, ATTRIB_POSITION, "aPosition" );
    glBindAttribLocation( m_ProgramObject, ATTRIB_NORMAL, "aNormal" );
    glBindAttribLocation( m_ProgramObject, ATTRIB_UV, "aUv" );

    // Link the program
    glLinkProgram ( m_ProgramObject );

    // Check the link status
    glGetProgramiv ( m_ProgramObject, GL_LINK_STATUS, &linked );

    if ( !linked )
    {
        GLint infoLen = 0;

        glGetProgramiv ( m_ProgramObject, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = new char[sizeof(char) * infoLen];

            glGetProgramInfoLog ( m_ProgramObject, infoLen, NULL, infoLog );
            P3D_LOGE( "Error linking program:\n%s", infoLog );

            delete[] infoLog;
        }

        glDeleteProgram ( m_ProgramObject );
        return;
    }

    // uniforms
    m_UniformMVP = glGetUniformLocation(m_ProgramObject, "uMVP");
    if (m_UniformMVP == -1) {
      P3D_LOGE("Could not bind uniform %s\n", "uMVP");
      return ;
    }

    // vertex array
    GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f,
                             -0.5f, -0.5f, 0.0f,
                             0.5f, -0.5f, 0.0f };

    glGenBuffers(1, &m_VertexPosObject);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexPosObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

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
    glViewport ( 0, 0, m_Width, m_Height );

    // Clear color, depth, stencil buffers
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
//    glDepthRangef(0.0f, 1.0f);
    //glFrontFace(GL_CW);
    //glEnable(GL_CULL_FACE);

    // Use the program object
    glUseProgram ( m_ProgramObject );

    if(m_ModelLoader->isLoaded() && m_ModelLoader->boundingRadius() > 0.0f)
    {
        // MVP
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, m_ModelLoader->boundingRadius() * 3.0f),
                                     glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(25.0f * D2R, 1.0f * m_Width / m_Height, 1.0f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP = proj * view * model;
        glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, glm::value_ptr(MVP));

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
            glDrawElements(GL_TRIANGLES, m_ModelLoader->indexCount(ModelLoader::VT_POS_UV_NORM),
                           GL_UNSIGNED_INT, (GLvoid*)m_ModelLoader->indexOffset(ModelLoader::VT_POS_UV_NORM));
        }
        if(m_ModelLoader->indexCount(ModelLoader::VT_POS_NORM))
        {
            glDrawElements(GL_TRIANGLES, m_ModelLoader->indexCount(ModelLoader::VT_POS_NORM),
                           GL_UNSIGNED_INT, (GLvoid*) m_ModelLoader->indexOffset(ModelLoader::VT_POS_NORM));
        }
    }
    else
    {
        glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

        // Load the vertex data
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexPosObject);
        glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(ATTRIB_POSITION);

        glDrawArrays ( GL_TRIANGLES, 0, 3 );
    }
}

bool P3dViewer::loadModel(const char *binaryData, size_t size)
{
    bool res = m_ModelLoader->load(binaryData, size);
    return res;
}


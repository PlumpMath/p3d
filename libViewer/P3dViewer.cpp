#include "P3dViewer.h"
#include "PlatformAdapter.h"
#include <stdio.h>
#include "glwrapper.h"

P3dViewer::P3dViewer(PlatformAdapter* adapter)
{
    m_Adapter = adapter;
    if(!m_Adapter)
    {
        m_Adapter = new PlatformAdapter();
    }

    m_ProgramObject = 0;
    m_VertexPosObject = 0;
    m_InitOk = false;
}

P3dViewer::~P3dViewer()
{
    delete m_Adapter;
}

GLuint P3dViewer::loadShader (GLenum type, const char *shaderSrc)
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
            printf( "Error compiling shader:\n%s\n", infoLog );
            fflush(stdout);

            delete[] infoLog;
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;
}

GLuint P3dViewer::loadShaderFromFile (GLenum type, const char *shaderFile)
{
    long size;
    FILE* f = fopen(shaderFile, "rb");
    if(!f) {
        printf("Unable to load shader: %s\n", shaderFile);
        fflush(stdout);
        return 0;
    }
    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *shaderSrc = new char[size + 1];
    fread(shaderSrc, size, 1, f);
    fclose(f);
    shaderSrc[size] = 0;

    GLuint shader = loadShader(type, shaderSrc);
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

    // Bind vPosition to attribute 0
    glBindAttribLocation ( m_ProgramObject, 0, "vPosition" );

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
            printf ( "Error linking program:\n%s\n", infoLog );
            fflush(stdout);

            delete[] infoLog;
        }

        glDeleteProgram ( m_ProgramObject );
        return;
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

void P3dViewer::onSurfaceChanged() {
    // No-op
}

void P3dViewer::drawFrame(int width, int height) {
    if(!m_InitOk) {
        return;
    }

    // Set the viewport
    glViewport ( 0, 0, width, height );

    // Clear the color buffer
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear ( GL_COLOR_BUFFER_BIT );

    // Use the program object
    glUseProgram ( m_ProgramObject );

    // Load the vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexPosObject);
    glVertexAttribPointer(0 /* ? */, 3, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(0);

    glDrawArrays ( GL_TRIANGLES, 0, 3 );
}


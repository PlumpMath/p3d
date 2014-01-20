#include "P3dViewer.h"
#include "glwrapper.h"
#include <stdlib.h>
#include <stdio.h>

P3dViewer::P3dViewer()
{
}

GLuint programObject;
GLuint vertexPosObject;
int initOk = 0;

GLuint LoadShader (GLenum type, const char *shaderSrc)
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
//            char* infoLog = new char[sizeof(char) * infoLen ];
            char* infoLog = (char*) malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog ( shader, infoLen, 0, infoLog );
            printf( "Error compiling shader:\n%s\n", infoLog );
            fflush(stdout);

//            delete[] infoLog;
            free(infoLog);
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;
}

GLuint LoadShaderFromFile (GLenum type, const char *shaderFile)
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
//    char *shaderSrc = new char[size];
    char *shaderSrc = (char*) malloc(size + 1);
    fread(shaderSrc, size, 1, f);
    fclose(f);
    shaderSrc[size] = 0;

    GLuint shader = LoadShader(type, shaderSrc);
//    delete[] shaderSrc;
    free(shaderSrc);
    return shader;
}

void on_surface_created() {

    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = LoadShaderFromFile ( GL_VERTEX_SHADER, "shaders/vertex.glsl" );
    fragmentShader = LoadShaderFromFile ( GL_FRAGMENT_SHADER, "shaders/fragment.glsl" );

    // Create the program object
    programObject = glCreateProgram ( );

    if ( programObject == 0 )
       return;

    glAttachShader ( programObject, vertexShader );
    glAttachShader ( programObject, fragmentShader );

    // Bind vPosition to attribute 0
    glBindAttribLocation ( programObject, 0, "vPosition" );

    // Link the program
    glLinkProgram ( programObject );

    // Check the link status
    glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

    if ( !linked )
    {
        GLint infoLen = 0;

        glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
//            char* infoLog = new char[sizeof(char) * infoLen];
            char* infoLog = (char*) malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
            printf ( "Error linking program:\n%s\n", infoLog );
            fflush(stdout);

//            delete[] infoLog;
            free(infoLog);
        }

        glDeleteProgram ( programObject );
        return;
    }


    // vertex array
    GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f,
                             -0.5f, -0.5f, 0.0f,
                             0.5f, -0.5f, 0.0f };

    glGenBuffers(1, &vertexPosObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

    initOk = 1;
}

void on_surface_changed() {
    // No-op
}

void on_draw_frame(int width, int height) {
    if(!initOk) {
        return;
    }

    // Set the viewport
    glViewport ( 0, 0, width, height );

    // Clear the color buffer
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear ( GL_COLOR_BUFFER_BIT );

    // Use the program object
    glUseProgram ( programObject );

    // Load the vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
    glVertexAttribPointer(0 /* ? */, 3, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(0);

    glDrawArrays ( GL_TRIANGLES, 0, 3 );
}


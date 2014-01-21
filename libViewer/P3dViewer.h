#ifndef P3DVIEWER_H
#define P3DVIEWER_H

typedef unsigned int	GLuint;
typedef unsigned int	GLenum;

class PlatformAdapter;

void on_surface_created();
void on_surface_changed();
void on_draw_frame(int width, int height);

class P3dViewer
{
public:
    P3dViewer(PlatformAdapter* adapter = 0);
    virtual ~P3dViewer();
    void onSurfaceCreated();
    void onSurfaceChanged();
    void drawFrame(int width, int height);

private:
    GLuint loadShader (GLenum type, const char *shaderSrc);
    GLuint loadShaderFromFile (GLenum type, const char *shaderFile);


    PlatformAdapter* m_Adapter;
    GLuint m_ProgramObject;
    GLuint m_VertexPosObject;
    bool m_InitOk;
};

#endif // P3DVIEWER_H

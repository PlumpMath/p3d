#include <stdlib.h>
#include <stdio.h>
#include <GL/glfw.h>
#include <emscripten/emscripten.h>
#include "P3dViewer.h"

const int width = 480;
const int height = 800;


int init_gl(void);
void do_frame();
void shutdown_gl();

P3dViewer viewer;

void do_frame2(void* arg)
{
    (void) arg;
    do_frame();
}

int main(void) 
{
    if (init_gl() == GL_TRUE) {
        viewer.onSurfaceCreated();
        viewer.onSurfaceChanged();
        //emscripten_async_call(do_frame2, 0, 1);
        emscripten_set_main_loop(do_frame, 0, 1);
    }

    //shutdown_gl();

    return 0;
}

int init_gl()
{
    if (glfwInit() != GL_TRUE) {
        printf("glfwInit() failed\n");
        return GL_FALSE;
    }

    if (glfwOpenWindow(width, height, 8, 8, 8, 8, 16, 0, GLFW_WINDOW) != GL_TRUE) {
        printf("glfwOpenWindow() failed\n");
        return GL_FALSE;
    }

    return GL_TRUE;
}

void do_frame()
{
    viewer.drawFrame(width, height);
    glfwSwapBuffers();
}

void shutdown_gl()
{
    glfwTerminate();
}


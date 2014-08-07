#include <stdlib.h>
#include <stdio.h>
#include <GL/glfw.h>
#include <emscripten/emscripten.h>
#include "P3dViewer.h"
#include "CameraNavigation.h"
#include "PlatformAdapter.h"

static P3dLogger logger("em.main");

const int width = 853;
const int height = 480;


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
    logger.debug("running main");
    if (init_gl() == GL_TRUE) {
        viewer.onSurfaceCreated();
        viewer.onSurfaceChanged(width, height);
        //emscripten_async_call(do_frame2, 0, 1);
        emscripten_set_main_loop(do_frame, 0, 1);
    }

    //shutdown_gl();

    return 0;
}

void resize_gl(int width, int height )
{
    logger.debug("resize: %d x %d", width, height);
    viewer.onSurfaceChanged(width, height);
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

    glfwSetWindowSizeCallback(resize_gl);

    return GL_TRUE;
}

void do_frame()
{
    viewer.drawFrame();
    glfwSwapBuffers();
}

void shutdown_gl()
{
    glfwTerminate();
}

extern "C" void loadModel(const char* data, int size, const char* extension)
{
    viewer.loadModel(data, size, extension);
}

extern "C" void startRotateCam(float x, float y)
{
    viewer.cameraNavigation()->startRotate(x, y);
}

extern "C" void rotateCam(float x, float y)
{
    viewer.cameraNavigation()->rotate(x, y);
}

extern "C" void resetCam()
{
    viewer.cameraNavigation()->reset();
}

extern "C" int materialCount()
{
    return viewer.materialCount();
}

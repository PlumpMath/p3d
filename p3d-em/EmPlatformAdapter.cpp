#include "EmPlatformAdapter.h"
#include <emscripten/emscripten.h>
#include <GL/gl.h>

static P3dLogger logger("em.EmPlatformAdapter");

typedef void (*p3d_load_texture_onload_func)(void*, int);

extern "C" void p3d_load_texture(void* arg, const char* url, p3d_load_texture_onload_func onload);
extern "C" void p3d_cancel_textures();

struct PendingTex
{
    std::function<void(uint32_t)> callback;
};

static void tex_onload(void* arg, int texId)
{
    logger.debug("c++ got texId %d", texId);
    PendingTex* pending = static_cast<PendingTex*>(arg);
    if(texId)
    {
        pending->callback(texId);
    }
    delete pending;
}

EmPlatformAdapter::EmPlatformAdapter()
{
}

void EmPlatformAdapter::loadTexture(const char* name, std::function<void(uint32_t)> callback)
{
    logger.debug("load tex: %s", name);
    PendingTex* pending = new PendingTex();
    pending->callback = callback;
    p3d_load_texture(pending, name, tex_onload);
}

void EmPlatformAdapter::cancelTextureLoads()
{
    logger.debug("Cancel Textures");
    p3d_cancel_textures();
}

void EmPlatformAdapter::deleteTexture(uint32_t textureId)
{
    logger.debug("delete texture id %d", textureId);
    glDeleteTextures(1, &textureId);
}


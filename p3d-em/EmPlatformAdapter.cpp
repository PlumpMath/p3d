#include "EmPlatformAdapter.h"
#include <emscripten/emscripten.h>

static P3dLogger logger("em.EmPlatformAdapter");

typedef void (*p3d_load_texture_onload_func)(int);

extern "C" void p3d_load_texture(const char* url, p3d_load_texture_onload_func onload);

static std::function<void(uint32_t)> tex_callback;

static void tex_onload(int texId)
{
    logger.debug("c++ got texId %d", texId);
    tex_callback(texId);
}

EmPlatformAdapter::EmPlatformAdapter()
{
}

void EmPlatformAdapter::loadTexture(const char* name, std::function<void(uint32_t)> callback)
{
    logger.debug("load tex: %s", name);
    tex_callback = callback;
    p3d_load_texture(name, tex_onload);
}


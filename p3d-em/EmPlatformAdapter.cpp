#include "EmPlatformAdapter.h"

static P3dLogger logger("em.EmPlatformAdapter");

EmPlatformAdapter::EmPlatformAdapter()
{
}

void EmPlatformAdapter::loadTexture(const char* name, std::function<void(uint32_t)> callback)
{
    logger.debug("load tex: %s", name);
}


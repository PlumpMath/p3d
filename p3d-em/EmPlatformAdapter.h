#ifndef EMPLATFORMADAPTER_H
#define EMPLATFORMADAPTER_H

#include "PlatformAdapter.h"

class EmPlatformAdapter : public PlatformAdapter
{
public:
    EmPlatformAdapter();
    virtual void loadTexture(const char* name, std::function<void(uint32_t)> callback);
    virtual void cancelTextureLoads();
    virtual void deleteTexture(uint32_t textureId);
};

#endif // EMPLATFORMADAPTER_H

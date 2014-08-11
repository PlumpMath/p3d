#ifndef EMPLATFORMADAPTER_H
#define EMPLATFORMADAPTER_H

#include "PlatformAdapter.h"

class EmPlatformAdapter : public PlatformAdapter
{
public:
    EmPlatformAdapter();
    virtual void loadTexture(const char* name, std::function<void(uint32_t)> callback);
};

#endif // EMPLATFORMADAPTER_H

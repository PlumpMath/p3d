#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <cstdlib>

class ModelLoader
{
public:
    ModelLoader();
    virtual ~ModelLoader();
    bool load(const char* data, size_t size);
};

#endif // MODELLOADER_H

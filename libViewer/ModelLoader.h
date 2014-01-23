#ifndef MODELLOADER_H
#define MODELLOADER_H

class ModelLoader
{
public:
    ModelLoader();
    virtual ~ModelLoader();
    bool load(const char* data);
};

#endif // MODELLOADER_H

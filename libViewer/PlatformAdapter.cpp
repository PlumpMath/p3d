#include "PlatformAdapter.h"
#include <cstdio>

PlatformAdapter::PlatformAdapter()
{
}

PlatformAdapter::~PlatformAdapter()
{
}

const char *PlatformAdapter::loadAsset(const char *filename, size_t *size)
{
    size_t filesize;
    FILE* f = fopen(filename, "rb");
    if(!f) {
        printf("Unable to load asset: %s\n", filename);
        fflush(stdout);
        return 0;
    }
    fseek(f, 0L, SEEK_END);
    filesize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *data;
    if(size)
    {
        *size = filesize;
        data = new char[filesize];
    }
    else
    {
        data = new char[filesize + 1];
    }
    fread(data, filesize, 1, f);
    fclose(f);
    if(!size)
    {
        data[filesize] = 0;
    }
    return data;
}

#include "ModelLoader.h"
#include <cstdio>
#include <stdint.h>
#include <endian.h>

#define READ_U32(x) (le32toh(*((uint32_t*) &x)))


ModelLoader::ModelLoader()
{
}

ModelLoader::~ModelLoader()
{

}

bool ModelLoader::load(const char *data)
{
    printf("ModelLoader.load\n");
    fflush(stdout);

    uint32_t vcount = READ_U32(data[20]);
    uint32_t ncount = READ_U32(data[24]);
    uint32_t tcount = READ_U32(data[28]);
    printf("vcount %d\n", vcount);
    printf("ncount %d\n", ncount);
    printf("tcount %d\n", tcount);

    fflush(stdout);
    return true;
}

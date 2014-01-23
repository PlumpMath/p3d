#include "ModelLoader.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <endian.h>

#ifdef __ANDROID__
#define READ_U32(x) (letoh32(*((uint32_t*) &x)))
#else
#define READ_U32(x) (le32toh(*((uint32_t*) &x)))
#endif


ModelLoader::ModelLoader()
{
}

ModelLoader::~ModelLoader()
{

}

bool ModelLoader::load(const char *data, size_t size)
{
    printf("ModelLoader.load\n");
    fflush(stdout);

    // check header
    const char magic[] = "Three.js 003";
    size_t magicSize = sizeof(magic) - 1;
    if(size < magicSize)
    {
        // data too short
        return false;
    }
    if(strncmp(data, magic, magicSize))
    {
        // magic doesn't match
        return false;
    }
    uint8_t headerSize = data[12];
    if(headerSize != 64 || size < 64)
    {
        // wrong header size or data too short
        return false;
    }

    // byte sizes aren't checked, assumed to be:
    // pos: 4 bytes
    // norm: 1 byte
    // tex: 4 bytes
    // pos index: 4 bytes
    // norm index: 4 bytes
    // tex index: 4 bytes
    // mat index: 2 bytes

    // vertex data
    uint32_t posCount = READ_U32(data[20 + 0*4]);
    uint32_t normCount = READ_U32(data[20 + 1*4]);
    uint32_t texCount = READ_U32(data[20 + 2*4]);
    printf("posCount %d\n", posCount);
    printf("normCount %d\n", normCount);
    printf("texCount %d\n", texCount);

    // trinagle indices
    uint32_t f3_count_pos  = READ_U32(data[20 + 3*4]);
    uint32_t f3_count_pos_norm  = READ_U32(data[20 + 4*4]);
    uint32_t f3_count_pos_uv  = READ_U32(data[20 + 5*4]);
    uint32_t f3_count_pos_uv_norm  = READ_U32(data[20 + 6*4]);

    // quad indices
    uint32_t f4_count_pos  = READ_U32(data[20 + 7*4]);
    uint32_t f4_count_pos_norm  = READ_U32(data[20 + 8*4]);
    uint32_t f4_count_pos_uv  = READ_U32(data[20 + 9*4]);
    uint32_t f4_count_pos_uv_norm  = READ_U32(data[20 + 10*4]);

    printf("f3_count_pos: %d\n", f3_count_pos);
    printf("f3_count_pos_norm: %d\n", f3_count_pos_norm);
    printf("f3_count_pos_uv: %d\n", f3_count_pos_uv);
    printf("f3_count_pos_uv_norm: %d\n", f3_count_pos_uv_norm);

    printf("f4_count_pos: %d\n", f4_count_pos);
    printf("f4_count_pos_norm: %d\n", f4_count_pos_norm);
    printf("f4_count_pos_uv: %d\n", f4_count_pos_uv);
    printf("f4_count_pos_uv_norm: %d\n", f4_count_pos_uv_norm);

    fflush(stdout);
    return true;
}

#include "ModelLoader.h"
#include "PlatformAdapter.h"
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
    P3D_LOGD("Loading %d bytes", size);

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
    uint32_t pos_count = READ_U32(data[20 + 0*4]);
    uint32_t norm_count = READ_U32(data[20 + 1*4]);
    uint32_t tex_count = READ_U32(data[20 + 2*4]);
    P3D_LOGD("pos_count %d", pos_count);
    P3D_LOGD("norm_count %d", norm_count);
    P3D_LOGD("tex_count %d", tex_count);

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

    P3D_LOGD("f3_count_pos: %d", f3_count_pos);
    P3D_LOGD("f3_count_pos_norm: %d", f3_count_pos_norm);
    P3D_LOGD("f3_count_pos_uv: %d", f3_count_pos_uv);
    P3D_LOGD("f3_count_pos_uv_norm: %d", f3_count_pos_uv_norm);

    P3D_LOGD("f4_count_pos: %d", f4_count_pos);
    P3D_LOGD("f4_count_pos_norm: %d", f4_count_pos_norm);
    P3D_LOGD("f4_count_pos_uv: %d", f4_count_pos_uv);
    P3D_LOGD("f4_count_pos_uv_norm: %d", f4_count_pos_uv_norm);

    // vertex data offsets
    uint32_t offset = headerSize;
    uint32_t pos_start = offset;
    offset += (3 * 4) * pos_count;

    uint32_t norm_start = offset;
    offset += addPadding(3 * norm_count);

    uint32_t tex_start = offset;
    offset += (2 * 4) * tex_count;

    // triangle indices offsets
    uint32_t f3_start_pos_start = offset;
    offset += addPadding((3 * 4 + 2) * f3_count_pos);

    uint32_t f3_start_pos_norm = offset;
    offset += addPadding((3 * (4 + 4) + 2) * f3_count_pos_norm);

    uint32_t f3_start_pos_uv = offset;
    offset += addPadding((3 * (4 + 4) + 2) * f3_count_pos_uv);

    uint32_t f3_start_pos_uv_norm = offset;
    offset += addPadding((3 * (4 + 4 + 4) + 2) * f3_count_pos_uv_norm);

    // quad indices offsets
    uint32_t f4_start_pos_start = offset;
    offset += addPadding((4 * 4 + 2) * f4_count_pos);

    uint32_t f4_start_pos_norm = offset;
    offset += addPadding((4 * (4 + 4) + 2) * f4_count_pos_norm);

    uint32_t f4_start_pos_uv = offset;
    offset += addPadding((4 * (4 + 4) + 2) * f4_count_pos_uv);

    uint32_t f4_start_pos_uv_norm = offset;
    offset += addPadding((4 * (4 + 4 + 4) + 2) * f4_count_pos_uv_norm);

    if(offset != size)
    {
        // wrong data size or some error in header
        return false;
    }

    return true;
}

size_t ModelLoader::addPadding(size_t size)
{
    return size + ( ( size % 4 ) ? ( 4 - size % 4 ) : 0 );
}

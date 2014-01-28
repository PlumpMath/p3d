#include "ModelLoader.h"
#include "PlatformAdapter.h"
#include "glwrapper.h"
#include <cstring>
#include <endian.h>
#include <unordered_map>
#include <vector>

#ifdef __ANDROID__
#define READ_U32(x) (letoh32(*((uint32_t*) &x)))
#define READ_U16(x) (letoh16(*((uint16_t*) &x)))
#else
#define READ_U32(x) (le32toh(*((uint32_t*) &x)))
#define READ_U16(x) (le16toh(*((uint16_t*) &x)))
#endif

float READ_FLOAT(const char& x) {
    uint32_t val = READ_U32(x);
    return *((float*) &val);
}

struct IndexPosUvNorm
{
    uint32_t pos;
    uint32_t uv;
    uint32_t norm;

    bool operator==(const IndexPosUvNorm &other) const
    {
        return memcmp(this, &other, sizeof(*this)) == 0;
    }
};

namespace std {
template <> struct hash<IndexPosUvNorm>
{
    size_t operator()(const IndexPosUvNorm& k) const
    {
        size_t h1 = k.pos;
        size_t h2 = k.uv;
        h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        h2 = k.norm;
        h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        return h1;
    }
};
}

typedef std::unordered_map<IndexPosUvNorm, uint32_t> IndexPosUvNormMap;




ModelLoader::ModelLoader()
{
    m_data = 0;
    m_loaded = false;
}

ModelLoader::~ModelLoader()
{

}

bool ModelLoader::load(const char *data, size_t size)
{
    m_loaded = false;
    m_data = data;

    P3D_LOGD("Loading %d bytes", size);

    // check header
    const char magic[] = "Three.js 003";
    size_t magicSize = sizeof(magic) - 1;
    if(size < magicSize)
    {
        P3D_LOGE("data too short");
        return false;
    }
    if(strncmp(data, magic, magicSize))
    {
        P3D_LOGE("magic doesn't match");
        return false;
    }
    uint8_t headerSize = data[12];
    if(headerSize != 64 || size < 64)
    {
        P3D_LOGE("wrong header size or data too short");
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
    m_pos_count = READ_U32(data[20 + 0*4]);
    m_norm_count = READ_U32(data[20 + 1*4]);
    m_tex_count = READ_U32(data[20 + 2*4]);
    P3D_LOGD("pos_count %d", m_pos_count);
    P3D_LOGD("norm_count %d", m_norm_count);
    P3D_LOGD("tex_count %d", m_tex_count);

    // trinagle indices
    m_f3_count_pos  = READ_U32(data[20 + 3*4]);
    m_f3_count_pos_norm  = READ_U32(data[20 + 4*4]);
    m_f3_count_pos_uv  = READ_U32(data[20 + 5*4]);
    m_f3_count_pos_uv_norm  = READ_U32(data[20 + 6*4]);

    // quad indices
    m_f4_count_pos  = READ_U32(data[20 + 7*4]);
    m_f4_count_pos_norm  = READ_U32(data[20 + 8*4]);
    m_f4_count_pos_uv  = READ_U32(data[20 + 9*4]);
    m_f4_count_pos_uv_norm  = READ_U32(data[20 + 10*4]);

    P3D_LOGD("f3_count_pos: %d", m_f3_count_pos);
    P3D_LOGD("f3_count_pos_norm: %d", m_f3_count_pos_norm);
    P3D_LOGD("f3_count_pos_uv: %d", m_f3_count_pos_uv);
    P3D_LOGD("f3_count_pos_uv_norm: %d", m_f3_count_pos_uv_norm);

    P3D_LOGD("f4_count_pos: %d", m_f4_count_pos);
    P3D_LOGD("f4_count_pos_norm: %d", m_f4_count_pos_norm);
    P3D_LOGD("f4_count_pos_uv: %d", m_f4_count_pos_uv);
    P3D_LOGD("f4_count_pos_uv_norm: %d", m_f4_count_pos_uv_norm);

    // vertex data offsets
    uint32_t offset = headerSize;
    m_pos_start = offset;
    offset += (3 * 4) * m_pos_count;

    m_norm_start = offset;
    offset += addPadding(3 * m_norm_count);

    m_tex_start = offset;
    offset += (2 * 4) * m_tex_count;

    // triangle indices offsets
    m_f3_start_pos = offset;
    offset += addPadding((3 * 4 + 2) * m_f3_count_pos);

    m_f3_start_pos_norm = offset;
    offset += addPadding((3 * (4 + 4) + 2) * m_f3_count_pos_norm);

    m_f3_start_pos_uv = offset;
    offset += addPadding((3 * (4 + 4) + 2) * m_f3_count_pos_uv);

    m_f3_start_pos_uv_norm = offset;
    offset += addPadding((3 * (4 + 4 + 4) + 2) * m_f3_count_pos_uv_norm);

    // quad indices offsets
    m_f4_start_pos = offset;
    offset += addPadding((4 * 4 + 2) * m_f4_count_pos);

    m_f4_start_pos_norm = offset;
    offset += addPadding((4 * (4 + 4) + 2) * m_f4_count_pos_norm);

    m_f4_start_pos_uv = offset;
    offset += addPadding((4 * (4 + 4) + 2) * m_f4_count_pos_uv);

    m_f4_start_pos_uv_norm = offset;
    offset += addPadding((4 * (4 + 4 + 4) + 2) * m_f4_count_pos_uv_norm);

    if(offset != size)
    {
        // wrong data size or some error in header
        return false;
    }

    deindex(data);

    m_loaded = true;
    return true;
}

float ModelLoader::boundingRadius()
{
    return std::max(m_maxX - m_minX, std::max(m_maxY - m_minY, m_maxZ - m_minZ));
}

size_t ModelLoader::addPadding(size_t size)
{
    return size + ( ( size % 4 ) ? ( 4 - size % 4 ) : 0 );
}

void ModelLoader::deindex(const char* data)
{
    uint16_t maxMat = 0;
    uint16_t mat;
    uint32_t i;
    uint32_t il;
    uint32_t f;
    uint32_t faces;
    uint32_t pos_offset;
    uint32_t uv_offset;
    uint32_t norm_offset;
    uint32_t mat_offset;
    uint32_t new_offset;
    uint32_t new_mat_offset;
    uint32_t vert_offset;
    uint32_t f4_offset;
    float x;
    float y;
    float z;
    m_maxX = 0.0f;
    m_minX = 0.0f;
    m_maxY = 0.0f;
    m_minY = 0.0f;
    m_maxZ = 0.0f;
    m_minZ = 0.0f;


    static const float norm_scale = 1.0f / 127.0f;

    std::vector<GLfloat> new_pos;
    new_pos.reserve(m_pos_count);

    std::vector<GLfloat> new_uv;
    new_uv.reserve(m_tex_count);

    std::vector<GLfloat> new_norm;
    new_norm.reserve(m_norm_count);

    //TODO: other index types

    // pos uv norm
    IndexPosUvNormMap map;
    m_index_count_pos_uv_norm = m_f3_count_pos_uv_norm * 3 +
            m_f4_count_pos_uv_norm * 6;
    uint32_t* new_pos_uv_norm = new uint32_t[m_index_count_pos_uv_norm];
    uint16_t* new_mat_pos_uv_norm = new uint16_t[m_f3_count_pos_uv_norm +
            m_f4_count_pos_uv_norm * 2];

    // tris
    pos_offset = m_f3_start_pos_uv_norm;
    norm_offset = pos_offset + m_f3_count_pos_uv_norm * 3 * 4;
    uv_offset = norm_offset + m_f3_count_pos_uv_norm * 3 * 4;
    mat_offset = uv_offset + m_f3_count_pos_uv_norm * 3 * 4;

    IndexPosUvNorm index;
    uint32_t new_index;
    new_offset = 0;
    new_mat_offset = 0;
    f4_offset = m_f3_count_pos_uv_norm;
    for(i = 0, il = f4_offset + m_f4_count_pos_uv_norm; i < il; ++i)
    {
        if(i == f4_offset)
        {
            // quads
            pos_offset = m_f4_start_pos_uv_norm;
            norm_offset = pos_offset + m_f4_count_pos_uv_norm * 4 * 4;
            uv_offset = norm_offset + m_f4_count_pos_uv_norm * 4 * 4;
            mat_offset = uv_offset + m_f4_count_pos_uv_norm * 4 * 4;
        }

        faces = i < f4_offset ? 3 : 4;
        for(f = 0; f < faces; ++f)
        {
            index.pos = READ_U32(data[pos_offset]);
            pos_offset += 4;
            index.uv = READ_U32(data[uv_offset]);
            uv_offset += 4;
            index.norm = READ_U32(data[norm_offset]);
            norm_offset += 4;
            if(map.count(index))
            {
                new_index = map[index];
            }
            else
            {
                new_index = map.size();
                map.insert(std::pair<IndexPosUvNorm, uint32_t>(index, new_index));

                vert_offset = m_pos_start + 4 * (3 * index.pos);
                x = READ_FLOAT(data[vert_offset]);
                vert_offset += 4;
                y = READ_FLOAT(data[vert_offset]);
                vert_offset += 4;
                z = READ_FLOAT(data[vert_offset]);
                if(x > m_maxX) m_maxX = x;
                if(x < m_minX) m_minX = x;
                if(y > m_maxY) m_maxY = y;
                if(y < m_minY) m_minY = y;
                if(z > m_maxZ) m_maxZ = z;
                if(z < m_minZ) m_minZ = z;
                new_pos.push_back(x);
                new_pos.push_back(y);
                new_pos.push_back(z);

                vert_offset = m_tex_start + 4 * (2 * index.uv);
                new_uv.push_back(READ_FLOAT(data[vert_offset]));
                vert_offset += 4;
                new_uv.push_back(READ_FLOAT(data[vert_offset]));

                vert_offset = m_norm_start + (3 * index.norm);
                new_norm.push_back(static_cast<signed char>(data[vert_offset]) * norm_scale);
                vert_offset += 1;
                new_norm.push_back(static_cast<signed char>(data[vert_offset]) * norm_scale);
                vert_offset += 1;
                new_norm.push_back(static_cast<signed char>(data[vert_offset]) * norm_scale);
            }
            new_pos_uv_norm[new_offset] = new_index;
            ++new_offset;
        }

        // material
        mat = READ_U16(data[mat_offset]);
        mat_offset += 2;
        new_mat_pos_uv_norm[new_mat_offset++] = mat;
        if(mat > maxMat)
        {
            maxMat = mat;
        }

        // extra tri for quads
        if(faces == 4)
        {
            new_pos_uv_norm[new_offset] = new_pos_uv_norm[new_offset - 4];
            ++new_offset;
            new_pos_uv_norm[new_offset] = new_pos_uv_norm[new_offset - 3];
            ++new_offset;

            new_mat_pos_uv_norm[new_mat_offset++] = mat;
        }
    }

    m_mat_count = maxMat + 1;

    P3D_LOGD("mat count: %d", m_mat_count);
    P3D_LOGD("new pos size: %d", new_pos.size());
    P3D_LOGD("new uv size: %d", new_uv.size());
    P3D_LOGD("new norm size: %d", new_norm.size());

    // TODO: gen normals

    glGenBuffers(1, &m_pos_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_pos_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, new_pos.size() * sizeof(GLfloat), new_pos.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_uv_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_uv_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, new_uv.size() * sizeof(GLfloat), new_uv.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_norm_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_norm_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, new_norm.size() * sizeof(GLfloat), new_norm.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_index_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index_count_pos_uv_norm * sizeof(uint32_t), new_pos_uv_norm, GL_STATIC_DRAW);

    delete [] new_pos_uv_norm;
    delete [] new_mat_pos_uv_norm;
}


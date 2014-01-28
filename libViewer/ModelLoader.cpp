#include "ModelLoader.h"
#include "PlatformAdapter.h"
#include "glwrapper.h"
#include <cstring>
#include <endian.h>

#include "P3dVector.h"

#include "P3dMap.h"

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

bool ModelLoader::VertexIndex::operator==(const VertexIndex &other) const
{
    if(pos != other.pos) return false;
    switch(type)
    {
    case VT_POS_NORM:
        if(norm != other.norm) return false;
        break;
    case VT_POS_UV:
        if(uv != other.uv) return false;
        break;
    case VT_POS_UV_NORM:
        if(norm != other.norm) return false;
        if(uv != other.uv) return false;
        break;
    default:
        break;
    }

    return true;
}

size_t ModelLoader::VertexIndex::hash() const
{
    size_t h1 = pos;
    size_t h2 = uv;
    h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
    h2 = norm;
    h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
    return h1;
}


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
    m_f3_count[VT_POS]  = READ_U32(data[20 + 3*4]);
    m_f3_count[VT_POS_NORM]  = READ_U32(data[20 + 4*4]);
    m_f3_count[VT_POS_UV]  = READ_U32(data[20 + 5*4]);
    m_f3_count[VT_POS_UV_NORM]  = READ_U32(data[20 + 6*4]);

    // quad indices
    m_f4_count[VT_POS]  = READ_U32(data[20 + 7*4]);
    m_f4_count[VT_POS_NORM]  = READ_U32(data[20 + 8*4]);
    m_f4_count[VT_POS_UV]  = READ_U32(data[20 + 9*4]);
    m_f4_count[VT_POS_UV_NORM]  = READ_U32(data[20 + 10*4]);

    P3D_LOGD("f3_count[VT_POS]: %d", m_f3_count[VT_POS]);
    P3D_LOGD("f3_count[VT_POS_NORM]: %d", m_f3_count[VT_POS_NORM]);
    P3D_LOGD("f3_count[VT_POS_UV]: %d", m_f3_count[VT_POS_UV]);
    P3D_LOGD("f3_count[VT_POS_UV_NORM]: %d", m_f3_count[VT_POS_UV_NORM]);

    P3D_LOGD("f4_count[VT_POS]: %d", m_f4_count[VT_POS]);
    P3D_LOGD("f4_count[VT_POS_NORM]: %d", m_f4_count[VT_POS_NORM]);
    P3D_LOGD("f4_count[VT_POS_UV]: %d", m_f4_count[VT_POS_UV]);
    P3D_LOGD("f4_count[VT_POS_UV_NORM]: %d", m_f4_count[VT_POS_UV_NORM]);

    // vertex data offsets
    uint32_t offset = headerSize;
    m_pos_start = offset;
    offset += (3 * 4) * m_pos_count;

    m_norm_start = offset;
    offset += addPadding(3 * m_norm_count);

    m_tex_start = offset;
    offset += (2 * 4) * m_tex_count;

    // triangle indices offsets
    m_f3_start[VT_POS] = offset;
    offset += addPadding((3 * 4 + 2) * m_f3_count[VT_POS]);

    m_f3_start[VT_POS_NORM] = offset;
    offset += addPadding((3 * (4 + 4) + 2) * m_f3_count[VT_POS_NORM]);

    m_f3_start[VT_POS_UV] = offset;
    offset += addPadding((3 * (4 + 4) + 2) * m_f3_count[VT_POS_UV]);

    m_f3_start[VT_POS_UV_NORM] = offset;
    offset += addPadding((3 * (4 + 4 + 4) + 2) * m_f3_count[VT_POS_UV_NORM]);

    // quad indices offsets
    m_f4_start[VT_POS] = offset;
    offset += addPadding((4 * 4 + 2) * m_f4_count[VT_POS]);

    m_f4_start[VT_POS_NORM] = offset;
    offset += addPadding((4 * (4 + 4) + 2) * m_f4_count[VT_POS_NORM]);

    m_f4_start[VT_POS_UV] = offset;
    offset += addPadding((4 * (4 + 4) + 2) * m_f4_count[VT_POS_UV]);

    m_f4_start[VT_POS_UV_NORM] = offset;
    offset += addPadding((4 * (4 + 4 + 4) + 2) * m_f4_count[VT_POS_UV_NORM]);

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
    float xsize = m_maxX - m_minX;
    float ysize = m_maxY - m_minY;
    float zsize = m_maxZ - m_minZ;
    float result = xsize;
    if(ysize > result) result = ysize;
    if(zsize > result) result = zsize;
    return result;
}

size_t ModelLoader::addPadding(size_t size)
{
    return size + ( ( size % 4 ) ? ( 4 - size % 4 ) : 0 );
}

void ModelLoader::deindex(const char* data)
{
    m_mat_count = 1;
    m_maxX = 0.0f;
    m_minX = 0.0f;
    m_maxY = 0.0f;
    m_minY = 0.0f;
    m_maxZ = 0.0f;
    m_minZ = 0.0f;

    m_new_pos.clear();
    m_new_pos.reserve(m_pos_count);

    m_new_uv.clear();
    m_new_uv.reserve(m_tex_count);

    m_new_norm.clear();
    m_new_norm.reserve(m_norm_count);

    m_total_index_count = 0;

    for(int vtype = 0; vtype < 4; vtype++)
    {
        m_index_count[vtype] = m_f3_count[vtype] * 3 + m_f4_count[vtype] * 6;
        m_new_f3_start[vtype] = m_total_index_count;
        m_new_f4_start[vtype] = m_total_index_count + m_f3_count[vtype] * 3;
        m_total_index_count += m_index_count[vtype];
    }

    uint32_t* new_faces = new uint32_t[m_total_index_count];
    uint16_t* new_mats = new uint16_t[m_total_index_count / 3];

    m_vertex_map.clear();

    // pos uv norm
    deindexType(VT_POS_UV_NORM, data, new_faces, new_mats);
    deindexType(VT_POS_UV, data, new_faces, new_mats);
    deindexType(VT_POS_NORM, data, new_faces, new_mats);
    deindexType(VT_POS, data, new_faces, new_mats);

    //TODO: other index types


    P3D_LOGD("mat count: %d", m_mat_count);
    P3D_LOGD("new pos size: %d", m_new_pos.size());
    P3D_LOGD("new uv size: %d", m_new_uv.size());
    P3D_LOGD("new norm size: %d", m_new_norm.size());

    // TODO: gen normals

    glGenBuffers(1, &m_pos_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_pos_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_new_pos.size() * sizeof(GLfloat), m_new_pos.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_uv_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_uv_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_new_uv.size() * sizeof(GLfloat), m_new_uv.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_norm_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_norm_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_new_norm.size() * sizeof(GLfloat), m_new_norm.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_index_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_total_index_count * sizeof(uint32_t), new_faces, GL_STATIC_DRAW);

    delete [] new_faces;
    delete [] new_mats;
}

void ModelLoader::deindexType(ModelLoader::VertexType vtype, const char *data, uint32_t* new_faces, uint16_t* new_mats)
{
    uint32_t pos_offset;
    uint32_t uv_offset;
    uint32_t norm_offset;
    uint32_t mat_offset;
    uint16_t mat;
    uint32_t i;
    uint32_t il;
    uint32_t f;
    uint32_t faces;
    uint32_t new_offset;
    uint32_t new_mat_offset;
    uint32_t vert_offset;
    uint32_t f4_offset;
    float x;
    float y;
    float z;
    static const float norm_scale = 1.0f / 127.0f;

    // tris
    pos_offset = m_f3_start[vtype];
    norm_offset = pos_offset + m_f3_count[vtype] * 3 * 4;
    uv_offset = norm_offset + m_f3_count[vtype] * 3 * 4;
    mat_offset = uv_offset + m_f3_count[vtype] * 3 * 4;

    VertexIndex index;
    index.type = vtype;
    uint32_t new_index;
    new_offset = 0;
    new_mat_offset = 0;
    f4_offset = m_f3_count[vtype];
    for(i = 0, il = f4_offset + m_f4_count[vtype]; i < il; ++i)
    {
        if(i == f4_offset)
        {
            // quads
            pos_offset = m_f4_start[vtype];
            norm_offset = pos_offset + m_f4_count[vtype] * 4 * 4;
            uv_offset = norm_offset + m_f4_count[vtype] * 4 * 4;
            mat_offset = uv_offset + m_f4_count[vtype] * 4 * 4;
        }

        faces = i < f4_offset ? 3 : 4;
        for(f = 0; f < faces; ++f)
        {
            index.pos = READ_U32(data[pos_offset]);
            pos_offset += 4;
            if(vtype == VT_POS_UV || vtype == VT_POS_UV_NORM)
            {
                index.uv = READ_U32(data[uv_offset]);
                uv_offset += 4;
            }
            if(vtype == VT_POS_NORM || vtype == VT_POS_UV_NORM)
            {
                index.norm = READ_U32(data[norm_offset]);
                norm_offset += 4;
            }
            if(m_vertex_map.count(index))
            {
                new_index = m_vertex_map[index];
            }
            else
            {
                new_index = m_vertex_map.size();
                m_vertex_map.insert(index, new_index);
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
                m_new_pos.push_back(x);
                m_new_pos.push_back(y);
                m_new_pos.push_back(z);

                if(vtype == VT_POS_UV || vtype == VT_POS_UV_NORM)
                {
                    vert_offset = m_tex_start + 4 * (2 * index.uv);
                    m_new_uv.push_back(READ_FLOAT(data[vert_offset]));
                    vert_offset += 4;
                    m_new_uv.push_back(READ_FLOAT(data[vert_offset]));
                }

                if(vtype == VT_POS_NORM || vtype == VT_POS_UV_NORM)
                {
                    vert_offset = m_norm_start + (3 * index.norm);
                    m_new_norm.push_back(static_cast<signed char>(data[vert_offset]) * norm_scale);
                    vert_offset += 1;
                    m_new_norm.push_back(static_cast<signed char>(data[vert_offset]) * norm_scale);
                    vert_offset += 1;
                    m_new_norm.push_back(static_cast<signed char>(data[vert_offset]) * norm_scale);
                }
                else
                {
                    // push emtpy normal
                    m_new_norm.push_back(0.0f);
                    m_new_norm.push_back(0.0f);
                    m_new_norm.push_back(0.0f);
                }
            }
            new_faces[new_offset] = new_index;
            ++new_offset;
        }

        // material
        mat = READ_U16(data[mat_offset]);
        mat_offset += 2;
        new_mats[new_mat_offset++] = mat;
        if(mat + 1> m_mat_count)
        {
            m_mat_count = mat + 1;
        }

        // extra tri for quads
        if(faces == 4)
        {
            new_faces[new_offset] = new_faces[new_offset - 4];
            ++new_offset;
            new_faces[new_offset] = new_faces[new_offset - 3];
            ++new_offset;

            new_mats[new_mat_offset++] = mat;
        }
    }
}


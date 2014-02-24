#include "ModelLoader.h"
#include "PlatformAdapter.h"
#include "glwrapper.h"
#include <cstring>
#if defined(_WIN32) || defined(_WIN64)
static inline uint32_t le32toh(uint32_t x) {return x;}
static inline uint16_t le16toh(uint16_t x) {return x;}
#else
#include <endian.h>
#endif

#include "P3dVector.h"

#include "P3dMap.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

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
    if(type != other.type) return false;
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
    m_loaded = false;
}

ModelLoader::~ModelLoader()
{
    clear();
}

bool ModelLoader::load(const char *data, size_t size)
{
    P3D_LOGD("Loading %d bytes", size);

    clear();

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

    m_loaded = reindex(data);

    return m_loaded;
}

void ModelLoader::clear()
{
    if(m_loaded)
    {
        m_loaded = false;
        for(int i = 0, il = m_chunks.size(); i < il; ++i)
        {
            glDeleteBuffers(1, &m_chunks[i].posBuffer);
            glDeleteBuffers(1, &m_chunks[i].uvBuffer);
            glDeleteBuffers(1, &m_chunks[i].normBuffer);
        }
        m_chunks.clear();

        glDeleteBuffers(1, &m_index_buffer);

        for(int i = 0, il = m_vertex_maps.size(); i < il; ++i)
        {
            delete m_vertex_maps[i];
        }
        m_vertex_maps.clear();
    }
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

void ModelLoader::copyVertData(uint32_t chunk, const char* data, GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos)
{
    static const float norm_scale = 1.0f / 127.0f;
    uint32_t new_offset;
    uint32_t vert_offset;
    float x;
    float y;
    float z;
    int vertCount = 0;
    for(P3dMap<VertexIndex, uint32_t>::iterator itr = m_vertex_maps[chunk]->begin(); itr.hasNext(); ++itr)
    {
        const VertexIndex& index = itr.key();
        uint32_t new_index = itr.value();
        //P3D_LOGD("%d: %d/%d/%d > %d", vertCount, index.pos, index.uv, index.norm, new_index);

        ++vertCount;
        // pos
        new_offset = new_index * 3 + m_new_pos_offsets[chunk];
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
        new_pos[new_offset++] = x;
        new_pos[new_offset++] = y;
        new_pos[new_offset++] = z;

        // uv
        if(index.type == VT_POS_UV || index.type == VT_POS_UV_NORM)
        {
            new_offset = new_index * 2 + m_new_uv_offsets[chunk];
            vert_offset = m_tex_start + 4 * (2 * index.uv);
            new_uv[new_offset++] = READ_FLOAT(data[vert_offset]);
            vert_offset += 4;
            new_uv[new_offset++] = READ_FLOAT(data[vert_offset]);
        }

        // norm
        new_offset = new_index * 3 + m_new_norm_offsets[chunk];
        if(index.type == VT_POS_NORM || index.type == VT_POS_UV_NORM)
        {
            vert_offset = m_norm_start + (3 * index.norm);
            new_norm[new_offset++] = static_cast<signed char>(data[vert_offset]) * norm_scale;
            vert_offset += 1;
            new_norm[new_offset++] = static_cast<signed char>(data[vert_offset]) * norm_scale;
            vert_offset += 1;
            new_norm[new_offset++] = static_cast<signed char>(data[vert_offset]) * norm_scale;
        }
        else
        {
            // store emtpy normal
            new_norm[new_offset++] = 0.0f;
            new_norm[new_offset++] = 0.0f;
            new_norm[new_offset++] = 0.0f;
        }
    }
}

bool ModelLoader::reindex(const char* data)
{
    m_mat_count = 1;
    m_maxX = 0.0f;
    m_minX = 0.0f;
    m_maxY = 0.0f;
    m_minY = 0.0f;
    m_maxZ = 0.0f;
    m_minZ = 0.0f;

    m_new_pos_count = 0;
    m_new_norm_count = 0;
    m_new_uv_count = 0;

    m_total_index_count = 0;

    uint32_t chunk = 0;
    m_chunks[chunk] = MeshChunk();
    m_vertex_maps[chunk] = new P3dMap<VertexIndex, uint32_t>();
    m_new_pos_offsets[chunk] = 0;
    m_new_uv_offsets[chunk] = 0;
    m_new_norm_offsets[chunk] = 0;

    for(int vtype = 0; vtype < 4; vtype++)
    {
        m_chunks[chunk].index_count[vtype] = m_f3_count[vtype] * 3 + m_f4_count[vtype] * 6;
        m_chunks[chunk].f3_start[vtype] = m_total_index_count;
        m_chunks[chunk].f4_start[vtype] = m_total_index_count + m_f3_count[vtype] * 3;
        m_total_index_count += m_chunks[chunk].index_count[vtype];
    }

    uint16_t* new_faces = new uint16_t[m_total_index_count];
    uint16_t* new_mats = new uint16_t[m_total_index_count / 3];

    reindexType(chunk, VT_POS_UV_NORM, data, new_faces, new_mats);
    reindexType(chunk, VT_POS_UV, data, new_faces, new_mats);
    reindexType(chunk, VT_POS_NORM, data, new_faces, new_mats);
    reindexType(chunk, VT_POS, data, new_faces, new_mats);

    P3D_LOGD("mat count: %d", m_mat_count);
    P3D_LOGD("new pos size: %d", m_new_pos_count);
    P3D_LOGD("new uv size: %d", m_new_uv_count);
    P3D_LOGD("new norm size: %d", m_new_norm_count);

    P3D_LOGD("total new size: %d", (m_new_pos_count + m_new_norm_count + m_new_uv_count) * 4 + m_total_index_count * 2);


    GLfloat* new_pos = new GLfloat[m_new_pos_count];
    GLfloat* new_uv = new GLfloat[m_new_uv_count];
    GLfloat* new_norm = new GLfloat[m_new_norm_count];

    for(chunk = 0; chunk < m_chunks.size(); ++chunk)
    {
        P3D_LOGD("chunk: %d", chunk);
        P3D_LOGD(" index count: %d", m_chunks[chunk].index_count[VT_POS_UV_NORM]);
        P3D_LOGD(" vert count: %d", m_chunks[chunk].vertCount);
        P3D_LOGD(" f3 offset: %d", m_chunks[chunk].f3_start[VT_POS_UV_NORM]);
        P3D_LOGD(" f4 offset: %d", m_chunks[chunk].f4_start[VT_POS_UV_NORM]);
        m_vertex_maps[chunk]->dumpBucketLoad();

        copyVertData(chunk, data, new_norm, new_uv, new_pos);

        delete m_vertex_maps[chunk];
    }
    m_vertex_maps.clear();

    generateNormals(new_faces, new_pos, new_norm);

    for(chunk = 0; chunk < m_chunks.size(); ++chunk)
    {
        glGenBuffers(1, &m_chunks[chunk].posBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_chunks[chunk].posBuffer);
        glBufferData(GL_ARRAY_BUFFER, 3 * m_chunks[chunk].vertCount * sizeof(GLfloat),
                     new_pos + m_new_pos_offsets[chunk], GL_STATIC_DRAW);

        glGenBuffers(1, &m_chunks[chunk].uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_chunks[chunk].uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, 2 * m_chunks[chunk].vertCount * sizeof(GLfloat),
                     new_uv + m_new_uv_offsets[chunk], GL_STATIC_DRAW);

        glGenBuffers(1, &m_chunks[chunk].normBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_chunks[chunk].normBuffer);
        glBufferData(GL_ARRAY_BUFFER, 3 * m_chunks[chunk].vertCount * sizeof(GLfloat),
                     new_norm + m_new_norm_offsets[chunk], GL_STATIC_DRAW);
    }

    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_total_index_count * sizeof(uint16_t), new_faces, GL_STATIC_DRAW);
    GL_CHECK_ERROR;

    delete [] new_norm;
    delete [] new_uv;
    delete [] new_pos;

    delete [] new_mats;
    delete [] new_faces;

    return true;
}

uint32_t ModelLoader::reindexType(uint32_t &chunk, ModelLoader::VertexType vtype, const char *data,
                                  uint16_t* new_faces, uint16_t* new_mats)
{
    uint32_t pos_offset;
    uint32_t uv_offset;
    uint32_t norm_offset;
    uint32_t mat_offset;
    uint16_t mat;
    uint32_t f;
    uint32_t fl;
    uint32_t v;
    uint32_t verts;
    uint32_t new_offset;
    uint32_t new_mat_offset;
    uint32_t f4_offset;
    uint32_t result = 0;
    bool in_f4 = false;

    // tris
    pos_offset = m_f3_start[vtype];
    norm_offset = pos_offset + m_f3_count[vtype] * 3 * 4;
    if(vtype == VT_POS_NORM || vtype == VT_POS_UV_NORM)
    {
        uv_offset = norm_offset + m_f3_count[vtype] * 3 * 4;
    } else {
        uv_offset = norm_offset;
    }
    if(vtype == VT_POS_UV || vtype == VT_POS_UV_NORM)
    {
        mat_offset = uv_offset + m_f3_count[vtype] * 3 * 4;
    } else {
        mat_offset = uv_offset;
    }

    VertexIndex index;
    index.type = vtype;
    uint32_t new_index;
    new_offset = 0;
    new_mat_offset = 0;
    f4_offset = m_f3_count[vtype];

    for(f = 0, fl = f4_offset + m_f4_count[vtype]; f < fl; ++f)
    {
        if(!in_f4 && f >= f4_offset)
        {
            in_f4 = true;
            // quads
            pos_offset = m_f4_start[vtype];
            norm_offset = pos_offset + m_f4_count[vtype] * 4 * 4;
            if(vtype == VT_POS_NORM || vtype == VT_POS_UV_NORM)
            {
                uv_offset = norm_offset + m_f4_count[vtype] * 4 * 4;
            } else {
                uv_offset = norm_offset;
            }
            if(vtype == VT_POS_UV || vtype == VT_POS_UV_NORM)
            {
                mat_offset = uv_offset + m_f4_count[vtype] * 4 * 4;
            } else {
                mat_offset = uv_offset;
            }
        }

        verts = in_f4 ? 4 : 3;
        for(v = 0; v < verts; ++v)
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
            if(m_vertex_maps[chunk]->count(index))
            {
                new_index = (*m_vertex_maps[chunk])[index];
            }
            else
            {
                new_index = m_vertex_maps[chunk]->size();
                m_vertex_maps[chunk]->insert(index, new_index);

                m_new_pos_count += 3;

                // uv
                if(vtype == VT_POS_UV || vtype == VT_POS_UV_NORM)
                {
                    m_new_uv_count += 2;
                }

                m_new_norm_count += 3;
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
        if(verts == 4)
        {
            new_faces[new_offset] = new_faces[new_offset - 4];
            ++new_offset;
            new_faces[new_offset] = new_faces[new_offset - 3];
            ++new_offset;

            new_mats[new_mat_offset++] = mat;
        }

        if(m_vertex_maps[chunk]->size() > 65530)
        {
            // next chunk
            ++chunk;
            m_chunks[chunk] = MeshChunk();
            MeshChunk& newChunk = m_chunks[chunk];
            MeshChunk& oldChunk = m_chunks[chunk - 1];
            newChunk.f3_start[vtype] = new_offset;
            if(in_f4)
            {
                newChunk.f4_start[vtype] = new_offset;
            }
            else
            {
                newChunk.f4_start[vtype] = oldChunk.f4_start[vtype] - new_offset;
            }
            newChunk.index_count[vtype] = oldChunk.index_count[vtype] - (new_offset - oldChunk.f3_start[vtype]);
            oldChunk.index_count[vtype] = new_offset - oldChunk.f3_start[vtype];
            oldChunk.vertCount = (m_new_pos_count - m_new_pos_offsets[chunk - 1]) / 3;

            for(int t = vtype + 1; t < 4; ++t)
            {
                newChunk.index_count[t] = oldChunk.index_count[t];
                newChunk.f3_start[t] = oldChunk.f3_start[t];
                newChunk.f4_start[t] = oldChunk.f4_start[t];
            }

            m_vertex_maps[chunk] = new P3dMap<VertexIndex, uint32_t>();
            m_new_pos_offsets[chunk] = m_new_pos_count;
            m_new_uv_offsets[chunk] = m_new_uv_count;
            m_new_norm_offsets[chunk] = m_new_norm_count;
        }
    }

    m_chunks[chunk].vertCount = (m_new_pos_count - m_new_pos_offsets[chunk]) / 3;
    return result;
}

void ModelLoader::generateNormals(uint16_t *new_faces, GLfloat *new_pos, GLfloat *new_norm)
{
    P3D_LOGD("Generating normals");
    uint64_t start = PlatformAdapter::currentMillis();

    class vec3key : public glm::vec3
    {
    public:
        vec3key() : glm::vec3() {}
        vec3key(float x, float y, float z) : glm::vec3(x, y, z) {}
        size_t hash() const
        {
            size_t h1 = *((uint32_t*) &x);
            size_t h2 = *((uint32_t*) &y);
            h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
            h2 = *((uint32_t*) &z);
            h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
            return h1;
        }
    };

    uint32_t i;
    uint32_t il;

    uint32_t a;
    uint32_t b;
    uint32_t c;

    uint32_t a_offset;
    uint32_t b_offset;
    uint32_t c_offset;

    P3dMap<vec3key, glm::vec3> normalsMap;
    static const VertexType vtypes[] = {VT_POS, VT_POS_UV};

    // calc
    for(uint32_t chunk = 0, chunkl = m_chunks.size(); chunk < chunkl; ++chunk)
    {
        for(int t = 0; t < 2; t++) {
            VertexType vtype = vtypes[t];

            for(i = m_chunks[chunk].f3_start[vtype], il =  i + m_chunks[chunk].index_count[vtype]; i < il;)
            {
                a = new_faces[i++];
                b = new_faces[i++];
                c = new_faces[i++];
                a_offset = 3 * a + m_new_pos_offsets[chunk];
                b_offset = 3 * b + m_new_pos_offsets[chunk];
                c_offset = 3 * c + m_new_pos_offsets[chunk];
                vec3key posa(new_pos[a_offset], new_pos[a_offset + 1], new_pos[a_offset + 2]);
                vec3key posb(new_pos[b_offset], new_pos[b_offset + 1], new_pos[b_offset + 2]);
                vec3key posc(new_pos[c_offset], new_pos[c_offset + 1], new_pos[c_offset + 2]);
                glm::vec3 fnormal = glm::cross(posa - posb, posb - posc);
                if(fnormal.x && fnormal.y && fnormal.z) fnormal = glm::normalize(fnormal);
                normalsMap[posa] += fnormal;
                normalsMap[posb] += fnormal;
                normalsMap[posc] += fnormal;
            }
        }
    }
    P3D_LOGD("calc took: %lld", PlatformAdapter::durationMillis(start));

    start = PlatformAdapter::currentMillis();
    // normalize
    for(P3dMap<vec3key, glm::vec3>::iterator itr = normalsMap.begin(); itr.hasNext(); ++itr)
    {
        glm::vec3& normal = itr.value();
        normal = glm::normalize(normal);
    }
    P3D_LOGD("normalize took: %lld", PlatformAdapter::durationMillis(start));

    start = PlatformAdapter::currentMillis();
    // store new normals
    for(uint32_t chunk = 0, chunkl = m_chunks.size(); chunk < chunkl; ++chunk)
    {
        for(int t = 0; t < 2; t++) {
            VertexType vtype = vtypes[t];

            for(i = m_chunks[chunk].f3_start[vtype], il = i + m_chunks[chunk].index_count[vtype]; i < il; ++i)
            {
                a = new_faces[i];
                a_offset = 3 * a + m_new_pos_offsets[chunk];
                vec3key posa(new_pos[a_offset], new_pos[a_offset + 1], new_pos[a_offset + 2]);
                const glm::vec3& normal = normalsMap[posa];
                a_offset = 3 * a + m_new_norm_offsets[chunk];
                new_norm[a_offset] = normal.x;
                new_norm[a_offset + 1] = normal.y;
                new_norm[a_offset + 2] = normal.z;
            }
        }
    }
    P3D_LOGD("store took: %lld", PlatformAdapter::durationMillis(start));

    P3D_LOGD("Calculated %d new normals", normalsMap.size());
    normalsMap.dumpBucketLoad();
}


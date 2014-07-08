#include "BinLoader.h"

#include <cfloat>

#if defined(_WIN32) || defined(_WIN64)
static inline uint32_t le32toh(uint32_t x) {return x;}
static inline uint16_t le16toh(uint16_t x) {return x;}
#else
#include <endian.h>
#endif

#ifdef __ANDROID__
#define READ_U32(x) (letoh32(*((uint32_t*) &x)))
#define READ_U16(x) (letoh16(*((uint16_t*) &x)))
#else
#define READ_U32(x) (le32toh(*((uint32_t*) &x)))
#define READ_U16(x) (le16toh(*((uint16_t*) &x)))
#endif

static float READ_FLOAT(const char& x) {
    uint32_t val = READ_U32(x);
    return *((float*) &val);
}

#include "ModelLoader.h"

static BinLoader binLoader;
static RegisterLoader registerBinLoader(&binLoader, ".bin", 0);

BinLoader::BinLoader()
{
    m_loaded = false;
    m_minX = FLT_MAX;
    m_maxX = FLT_MIN;
    m_minY = FLT_MAX;
    m_maxY = FLT_MIN;
    m_minZ = FLT_MAX;
    m_maxZ = FLT_MIN;
}

BinLoader::~BinLoader()
{

}

bool BinLoader::load(const char *data, size_t size)
{
    P3D_LOGD("Loading %d bytes", size);

    m_modelLoader->clear();

    m_chunks.clear();

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
    m_modelLoader->setBoundingBox(m_minX, m_maxX, m_minY, m_maxY, m_minZ, m_maxZ);
    m_modelLoader->setIsLoaded(m_loaded);

    return m_loaded;
}

size_t BinLoader::addPadding(size_t size)
{
    return size + ( ( size % 4 ) ? ( 4 - size % 4 ) : 0 );
}

bool BinLoader::reindex(const char *data)
{
    uint64_t start = PlatformAdapter::currentMillis();

    m_mat_count = 1;
    m_maxX = 0.0f;
    m_minX = 0.0f;
    m_maxY = 0.0f;
    m_minY = 0.0f;
    m_maxZ = 0.0f;
    m_minZ = 0.0f;

    m_new_pos_count = 0;
    m_new_norm_count = 0;
    m_new_empty_norm_count = 0;
    m_new_uv_count = 0;

    m_total_index_count = 0;

    uint32_t chunk = 0;

    for(int vtype = 0; vtype < 4; vtype++)
    {
        m_new_index_count[vtype] = m_f3_count[vtype] * 3 + m_f4_count[vtype] * 6;
        m_new_f3_start[vtype] = m_total_index_count;
        m_new_f4_start[vtype] = m_total_index_count + m_f3_count[vtype] * 3;
        m_total_index_count += m_new_index_count[vtype];
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
        P3D_LOGD(" index count: %d", m_chunks[chunk].indexCount);
        P3D_LOGD(" vert count: %d", m_chunks[chunk].vertCount);
        P3D_LOGD(" f3 offset: %d", m_chunks[chunk].f3Offset);
        P3D_LOGD(" f4 offset: %d", m_chunks[chunk].f4Offset);
        P3D_LOGD(" material: %d", m_chunks[chunk].material);
    }

    for(P3dMap<uint32_t, P3dMap<VertexIndex, uint32_t>*>::iterator itr = m_vertex_maps.begin(); itr.hasNext(); ++itr)
    {
        P3D_LOGD("vertex bank:");
        P3D_LOGD(" offset: %d", itr.key());
        P3D_LOGD(" count: %d", itr.value()->size());
        copyVertData(itr.key(), itr.value(), data, new_norm, new_uv, new_pos);
        itr.value()->dumpBucketLoad();
        delete itr.value();
    }
    m_vertex_maps.clear();

    m_modelLoader->createModel(m_new_pos_count, m_new_norm_count, m_new_empty_norm_count, m_new_uv_count,
                new_pos, new_norm, new_uv, m_total_index_count,
                new_faces, m_chunks.size(), m_chunks.data());

    delete [] new_norm;
    delete [] new_uv;
    delete [] new_pos;

    delete [] new_mats;
    delete [] new_faces;

    P3D_LOGD("reindex took %lldms", PlatformAdapter::durationMillis(start));
    return true;

}

uint32_t BinLoader::reindexType(uint32_t &chunk, BaseLoader::VertexType vtype, const char *data, uint16_t *new_faces, uint16_t *new_mats)
{
    uint32_t pos_offset;
    uint32_t uv_offset;
    uint32_t norm_offset;
    uint32_t mat_offset;
    uint16_t mat;
    uint32_t f;
    uint32_t fcount;
    uint32_t v;
    uint32_t verts;
    uint32_t new_offset;
    uint32_t new_mat_offset;
    uint32_t f4_offset;
    uint32_t result = 0;
    P3dMap<VertexIndex, uint32_t>* vertexMap = 0;
    bool in_f4 = false;

    f4_offset = m_f3_count[vtype];
    fcount = f4_offset + m_f4_count[vtype];
    if(fcount == 0)
    {
        // no faces of this type
        return result;
    }

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
    new_offset = 0; //TODO: should be passed in?
    new_mat_offset = 0;

    for(f = 0; f < fcount; ++f)
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

        // material
        mat = READ_U16(data[mat_offset]);
        mat_offset += 2;
        new_mats[new_mat_offset++] = mat;
        if(mat + 1> m_mat_count)
        {
            m_mat_count = mat + 1;
        }

        if(f == 0)
        {
            nextChunk(chunk, vtype, in_f4, new_offset, m_new_pos_count / 3, true);
            m_chunks[chunk].material = mat;
            vertexMap = m_vertex_maps[m_chunks[chunk].vertOffset];
        }
        else if(mat != m_chunks[chunk].material)
        {
            nextChunk(chunk, vtype, in_f4, new_offset, m_chunks[chunk].vertOffset, false);
            m_chunks[chunk].material = mat;
            vertexMap = m_vertex_maps[m_chunks[chunk].vertOffset];
        }

        if(vertexMap->size() > 65530)
        {
            // next chunk
            nextChunk(chunk, vtype, in_f4, new_offset, m_new_pos_count / 3, false);
            m_chunks[chunk].material = mat;
            vertexMap = m_vertex_maps[m_chunks[chunk].vertOffset];
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
            if(vertexMap->count(index))
            {
                new_index = (*vertexMap)[index];
            }
            else
            {
                new_index = vertexMap->size();
                vertexMap->insert(index, new_index);

                m_new_pos_count += 3;

                // uv
                if(vtype == VT_POS_UV || vtype == VT_POS_UV_NORM)
                {
                    m_new_uv_count += 2;
                }

                m_new_norm_count += 3;
                if(vtype == VT_POS || vtype == VT_POS_UV)
                {
                    m_new_empty_norm_count += 3;
                }
            }
            new_faces[new_offset] = new_index;
            ++new_offset;
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
    }

    m_chunks[chunk].vertCount = (m_new_pos_count - 3 * m_chunks[chunk].vertOffset) / 3;
    return result;
}

void BinLoader::copyVertData(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t> *vertexMap, const char *data, GLfloat *new_norm, GLfloat *new_uv, GLfloat *new_pos)
{
    static const float norm_scale = 1.0f / 127.0f;
    uint32_t new_offset;
    uint32_t vert_offset;
    float x;
    float y;
    float z;
    int vertCount = 0;
    for(P3dMap<VertexIndex, uint32_t>::iterator itr = vertexMap->begin(); itr.hasNext(); ++itr)
    {
        const VertexIndex& index = itr.key();
        uint32_t new_index = itr.value();
        //P3D_LOGD("%d: %d/%d/%d > %d", vertCount, index.pos, index.uv, index.norm, new_index);

        ++vertCount;
        // pos
        new_offset = (new_index + vertOffset) * 3;
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
            new_offset = (new_index + vertOffset) * 2;
            vert_offset = m_tex_start + 4 * (2 * index.uv);
            new_uv[new_offset++] = READ_FLOAT(data[vert_offset]);
            vert_offset += 4;
            new_uv[new_offset++] = READ_FLOAT(data[vert_offset]);
        }

        // norm
        new_offset = (new_index + vertOffset) * 3;
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

void BinLoader::nextChunk(uint32_t &chunk, BaseLoader::VertexType vtype, bool in_f4, uint32_t new_offset, uint32_t vertOffset, bool firstOfType)
{
    if(m_chunks.size() != 0)
    {
        ++chunk;
    }
    m_chunks[chunk] = MeshChunk();
    MeshChunk& newChunk = m_chunks[chunk];
    newChunk.f3Offset = new_offset;
    newChunk.f4Offset = new_offset;

    newChunk.validNormals = vtype == VT_POS_NORM || vtype == VT_POS_UV_NORM;
    newChunk.hasUvs = vtype == VT_POS_UV || vtype == VT_POS_UV_NORM;

    newChunk.vertOffset = vertOffset;

    if(firstOfType)
    {
        newChunk.indexCount = m_new_index_count[vtype];
        newChunk.f3Offset = m_new_f3_start[vtype];
        newChunk.f4Offset = m_new_f4_start[vtype];
    }
    else
    {
        MeshChunk& oldChunk = m_chunks[chunk - 1];
        if(!in_f4)
        {
            newChunk.f4Offset = oldChunk.f4Offset - new_offset;
        }
        newChunk.indexCount = oldChunk.indexCount - (new_offset - oldChunk.f3Offset);
        oldChunk.indexCount = new_offset - oldChunk.f3Offset;
        oldChunk.vertCount = (m_new_pos_count - oldChunk.vertOffset * 3) / 3;
    }

    if(m_vertex_maps.count(newChunk.vertOffset) == 0)
    {
        m_vertex_maps[newChunk.vertOffset] = new P3dMap<VertexIndex, uint32_t>(8192);
    }
}

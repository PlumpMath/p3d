#include "ModelLoader.h"
#include "PlatformAdapter.h"
#include "glwrapper.h"
#include <cstring>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>


#define STRIDE 3

struct GLBuffers
{
    GLBuffers()
    {
        memset(this, 0, sizeof(GLBuffers));
    }

    GLuint posBuffer;
    GLuint uvBuffer;
    GLuint normBuffer;
    uint32_t vertCount;
};

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

template<>
class P3dHasher<glm::vec3>
{
public:
    size_t static hash(const glm::vec3& k)
    {
        size_t h1 = *((uint32_t*) &k.x);
        size_t h2 = *((uint32_t*) &k.y);
        h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        h2 = *((uint32_t*) &k.z);
        h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        return h1;
    }
};

ModelLoader::ModelLoader()
{
    m_loaded = false;
    m_minX = FLT_MAX;
    m_maxX = FLT_MIN;
    m_minY = FLT_MAX;
    m_maxY = FLT_MIN;
    m_minZ = FLT_MAX;
    m_maxZ = FLT_MIN;
}

ModelLoader::~ModelLoader()
{
    clear();
}

void ModelLoader::clear()
{
    if(m_loaded)
    {
        m_loaded = false;
        for(P3dMap<uint32_t, GLBuffers*>::iterator itr = m_gl_buffers.begin(); itr.hasNext(); ++itr)
        {
            glDeleteBuffers(1, &itr.value()->posBuffer);
            glDeleteBuffers(1, &itr.value()->uvBuffer);
            glDeleteBuffers(1, &itr.value()->normBuffer);
            delete itr.value();
        }
        m_gl_buffers.clear();

        m_chunks.clear();

        glDeleteBuffers(1, &m_index_buffer);

        for(P3dMap<uint32_t, P3dMap<VertexIndex, uint32_t>*>::iterator itr = m_vertex_maps.begin(); itr.hasNext(); ++itr)
        {
            delete itr.value();
        }
        m_vertex_maps.clear();
    }
}

GLuint ModelLoader::posBuffer(uint32_t chunk)
{
    return m_gl_buffers[m_chunks[chunk].vertOffset]->posBuffer;
}

GLuint ModelLoader::uvBuffer(uint32_t chunk)
{
    return m_gl_buffers[m_chunks[chunk].vertOffset]->uvBuffer;
}

GLuint ModelLoader::normBuffer(uint32_t chunk)
{
    return m_gl_buffers[m_chunks[chunk].vertOffset]->normBuffer;
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

void ModelLoader::setBoundingBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
    m_minX = minX;
    m_maxX = maxX;
    m_minY = minY;
    m_maxY = maxY;
    m_minZ = minZ;
    m_maxZ = maxZ;
}

size_t ModelLoader::addPadding(size_t size)
{
    return size + ( ( size % 4 ) ? ( 4 - size % 4 ) : 0 );
}

void ModelLoader::createModel(uint32_t posCount, uint32_t normCount, uint32_t emptyNormCount, uint32_t uvCount,
                              float* posBuffer, float* normBuffer, float* uvBuffer, uint32_t indexCount,
                              uint16_t* indexBuffer, uint32_t chunkCount, const MeshChunk* chunks)
{
    generateNormals(indexBuffer, posBuffer, normBuffer, emptyNormCount);
    uint32_t chunk;

    if(chunks)
    {
        for(chunk = 0; chunk < chunkCount; ++chunk)
        {
            m_chunks.push_back(chunks[chunk]);
        }
    }

    for(chunk = 0; chunk < m_chunks.size(); ++chunk)
    {
        if(m_gl_buffers.count(m_chunks[chunk].vertOffset) == 0)
        {
            GLBuffers* glbufs = new GLBuffers();
            glbufs->vertCount = m_chunks[chunk].vertCount;
            m_gl_buffers.insert(m_chunks[chunk].vertOffset, glbufs);
        }
        else
        {
            GLBuffers* glbufs = m_gl_buffers[m_chunks[chunk].vertOffset];
            if(m_chunks[chunk].vertCount > glbufs->vertCount)
            {
                glbufs->vertCount = m_chunks[chunk].vertCount;
            }
        }
    }

    for(P3dMap<uint32_t, GLBuffers*>::iterator itr = m_gl_buffers.begin(); itr.hasNext(); ++itr)
    {
        GLBuffers* glbufs = itr.value();

        P3D_LOGD("Creating gl buf, vertOffset: %d, vertCount: %d", itr.key(), glbufs->vertCount);

        if(3 * (itr.key() + glbufs->vertCount) <= posCount)
        {
            glGenBuffers(1, &glbufs->posBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, glbufs->posBuffer);
            glBufferData(GL_ARRAY_BUFFER, 3 * glbufs->vertCount * sizeof(GLfloat),
                         posBuffer + 3 * itr.key(), GL_STATIC_DRAW);
        }

        if(2 * (itr.key() + glbufs->vertCount) <= uvCount)
        {
            glGenBuffers(1, &glbufs->uvBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, glbufs->uvBuffer);
            glBufferData(GL_ARRAY_BUFFER, 2 * glbufs->vertCount * sizeof(GLfloat),
                         uvBuffer + 2 * itr.key(), GL_STATIC_DRAW);
        }

        if(3 * (itr.key() + glbufs->vertCount) <= normCount)
        {
            glGenBuffers(1, &glbufs->normBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, glbufs->normBuffer);
            glBufferData(GL_ARRAY_BUFFER, 3 * glbufs->vertCount * sizeof(GLfloat),
                         normBuffer + 3 * itr.key(), GL_STATIC_DRAW);
        }
    }

    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint16_t), indexBuffer, GL_STATIC_DRAW);
    GL_CHECK_ERROR;
}

void ModelLoader::generateNormals(uint16_t *new_faces, GLfloat *new_pos, GLfloat *new_norm, uint32_t emptyNormCount)
{
    P3D_LOGD("Generating normals");
    uint64_t start = PlatformAdapter::currentMillis();

    uint32_t i;
    uint32_t il;

    uint32_t a;
    uint32_t b;
    uint32_t c;

    uint32_t a_offset;
    uint32_t b_offset;
    uint32_t c_offset;

    P3dMap<glm::vec3, glm::vec3> normalsMap(emptyNormCount / 128);

    // calc
    for(uint32_t chunk = 0, chunkl = m_chunks.size(); chunk < chunkl; ++chunk)
    {
        if(m_chunks[chunk].validNormals)
        {
            continue;
        }

        for(i = m_chunks[chunk].f3Offset, il =  i + m_chunks[chunk].indexCount; i < il;)
        {
            a = new_faces[i++];
            b = new_faces[i++];
            c = new_faces[i++];
            a_offset = 3 * (a + m_chunks[chunk].vertOffset);
            b_offset = 3 * (b + m_chunks[chunk].vertOffset);
            c_offset = 3 * (c + m_chunks[chunk].vertOffset);
            glm::vec3 posa(new_pos[a_offset], new_pos[a_offset + 1], new_pos[a_offset + 2]);
            glm::vec3 posb(new_pos[b_offset], new_pos[b_offset + 1], new_pos[b_offset + 2]);
            glm::vec3 posc(new_pos[c_offset], new_pos[c_offset + 1], new_pos[c_offset + 2]);
            glm::vec3 fnormal = glm::cross(posa - posb, posb - posc);
            if(fnormal.x && fnormal.y && fnormal.z) fnormal = glm::normalize(fnormal);
            normalsMap[posa] += fnormal;
            normalsMap[posb] += fnormal;
            normalsMap[posc] += fnormal;
        }
    }
    P3D_LOGD("calc took: %lldms", PlatformAdapter::durationMillis(start));

    start = PlatformAdapter::currentMillis();
    // normalize
    for(P3dMap<glm::vec3, glm::vec3>::iterator itr = normalsMap.begin(); itr.hasNext(); ++itr)
    {
        glm::vec3& normal = itr.value();
        normal = glm::normalize(normal);
    }
    P3D_LOGD("normalize took: %lldms", PlatformAdapter::durationMillis(start));

    start = PlatformAdapter::currentMillis();
    // store new normals
    for(uint32_t chunk = 0, chunkl = m_chunks.size(); chunk < chunkl; ++chunk)
    {
        if(m_chunks[chunk].validNormals)
        {
            continue;
        }

        for(i = m_chunks[chunk].f3Offset, il = i + m_chunks[chunk].indexCount; i < il; ++i)
        {
            a = new_faces[i];
            a_offset = 3 * (a + m_chunks[chunk].vertOffset);
            glm::vec3 posa(new_pos[a_offset], new_pos[a_offset + 1], new_pos[a_offset + 2]);
            const glm::vec3& normal = normalsMap[posa];
            a_offset = 3 * (a + m_chunks[chunk].vertOffset);
            new_norm[a_offset] = normal.x;
            new_norm[a_offset + 1] = normal.y;
            new_norm[a_offset + 2] = normal.z;
        }

        m_chunks[chunk].validNormals = true;
    }
    P3D_LOGD("store took: %lldms", PlatformAdapter::durationMillis(start));

    P3D_LOGD("Calculated %d new normals", normalsMap.size());
    normalsMap.dumpBucketLoad();
}

/********** BLENDER DATA ***************************/

bool ModelLoader::load(const BlendData *blendData)
{
    uint64_t start = PlatformAdapter::currentMillis();
    uint32_t chunk = 0;

    uint16_t* new_faces = new uint16_t[blendData->totface*STRIDE];

    /* initialize counters and indices */
    m_new_pos_count = 0;
    m_new_norm_count = 0;
    m_new_uv_count = 0;
    m_total_index_count = 0;

    for(int vtype = 0; vtype < 4; vtype++)
    {
        m_new_index_count[vtype] = 0;
        m_new_f3_start[vtype] = 0;
        m_new_f4_start[vtype] = 0;
    }

    /* for now we do only VT_POS */
    m_new_index_count[VT_POS] = blendData->totface;
    m_new_f3_start[VT_POS] = 0;
    m_new_f4_start[VT_POS] = -1;
    m_total_index_count += m_new_index_count[VT_POS];

    /* reindex VT_POS */
    reindexTypeBlender(chunk, VT_POS, blendData, new_faces);

    GLfloat* new_pos = new GLfloat[blendData->totvert * STRIDE];
    GLfloat* new_uv = new GLfloat[blendData->totvert * STRIDE];
    GLfloat* new_norm = new GLfloat[blendData->totvert * STRIDE];

    if(new_pos==NULL || new_uv==NULL || new_norm==NULL) return false;

    P3D_LOGD("GLfloat buffers allocated");

    for(chunk = 0; chunk < m_chunks.size(); ++chunk)
    {
        P3D_LOGD("chunk: %d", chunk);
        P3D_LOGD(" index count: %d", m_chunks[chunk].indexCount);
        P3D_LOGD(" vert count: %d", m_chunks[chunk].vertCount);
        P3D_LOGD(" f3 offset: %d", m_chunks[chunk].f3Offset);
        P3D_LOGD(" f4 offset: %d", m_chunks[chunk].f4Offset);
        P3D_LOGD(" material: %d", m_chunks[chunk].material);
    }

    P3D_LOGD("----------");

    for(P3dMap<uint32_t, P3dMap<VertexIndex, uint32_t>*>::iterator itr = m_vertex_maps.begin(); itr.hasNext(); ++itr)
    {
        P3D_LOGD("vertex bank:");
        P3D_LOGD(" offset: %d", itr.key());
        P3D_LOGD(" count: %d", itr.value()->size());
        copyVertDataBlender(itr.key(), itr.value(), blendData, new_norm, new_uv, new_pos);
        itr.value()->dumpBucketLoad();
        delete itr.value();
    }

    P3D_LOGD("data copied");
    m_vertex_maps.clear();

    createModel(m_new_pos_count, m_new_norm_count, m_new_empty_norm_count, m_new_uv_count,
                new_pos, new_norm, new_uv, m_total_index_count,
                new_faces, m_chunks.size(), 0);

    delete [] new_norm;
    delete [] new_uv;
    delete [] new_pos;

    delete [] new_faces;

    P3D_LOGD("reindex took %lldms", PlatformAdapter::durationMillis(start));
    m_loaded = true;
    return true;

}

uint32_t ModelLoader::reindexTypeBlender(uint32_t &chunk, ModelLoader::VertexType vtype, const BlendData *blendData,
                                  uint16_t* new_faces)
{
    uint32_t pos_offset;
    uint16_t mat = 0;
    uint32_t face;
    uint32_t fcount;
    uint32_t vert;
    uint32_t verts;
    uint32_t new_offset;

    uint64_t start = PlatformAdapter::currentMillis();

    uint32_t result = 0;
    P3dMap<VertexIndex, uint32_t>* vertexMap = 0;
    bool in_quad = false;

    fcount = blendData->totface;

    if(fcount == 0)
    {
        // no faces of this type
        return result;
    }

#if 0
    P3D_LOGD("fcount: %u", fcount);


    float *vp = blendData->verts;
    for(uint i = 0; i < blendData->totvert; i++) {
        P3D_LOGD("v: %f %f %f", vp[i*3], vp[i*3+1], vp[i*3+2]);
    }

    P3D_LOGD("----------------");

    uint32_t *fp = blendData->faces;
    for(uint i = 0; i < blendData->totface; i++) {
        P3D_LOGD("f: %u %u %u", fp[i*3], fp[i*3+1], fp[i*3+2]);
    }

    P3D_LOGD("<<<<<<<<<<<<<<<");
#endif

    // tris
    pos_offset = 0;

    VertexIndex index;
    index.type = vtype;
    uint16_t new_index;
    new_offset = 0; //TODO: should be passed in?
    //new_mat_offset = 0;

    for(face = 0; face < fcount; ++face)
    {
        if(face == 0)
        {
            nextChunkBlender(chunk, vtype, in_quad, new_offset, m_new_pos_count / 3, true);
            m_chunks[chunk].material = mat;
            vertexMap = m_vertex_maps[m_chunks[chunk].vertOffset];
        }

        verts = in_quad ? 4 : 3;
        for(vert = 0; vert < verts; ++vert)
        {
            index.pos = blendData->faces[pos_offset];
            pos_offset++;

            /* TODO: norm */
            index.norm = 0;
            index.uv = 0;

            if(vertexMap->count(index))
            {
                new_index = (*vertexMap)[index];
            }
            else
            {
                new_index = vertexMap->size();
                vertexMap->insert(index, new_index);

                m_new_pos_count += 3;

                m_new_norm_count += 3;
                m_new_empty_norm_count += 3;
            }

            //P3D_LOGD("face %u, idx %u, new idx %u, %u", face, index.pos, new_index, vertexMap->size());
            new_faces[new_offset] = (uint16_t)new_index;
            ++new_offset;
        }
    }

    m_chunks[chunk].vertCount = (m_new_pos_count - STRIDE * m_chunks[chunk].vertOffset) / 3;

    P3D_LOGD("reindex type Blender took: %lldms", PlatformAdapter::durationMillis(start));

    return result;
}

void ModelLoader::nextChunkBlender(uint32_t &chunk, ModelLoader::VertexType vtype, bool in_f4, uint32_t new_offset,
                            uint32_t vertOffset, bool firstOfType)
{
    P3D_LOGD("new_offset: %d", new_offset);
    P3D_LOGD("vertOffset: %d", vertOffset);
    if(m_chunks.size() != 0)
    {
        ++chunk;
    }
    m_chunks[chunk] = MeshChunk();
    MeshChunk& newChunk = m_chunks[chunk];
    newChunk.indexCount = m_new_index_count[vtype];
    newChunk.f3Offset = new_offset;

    newChunk.validNormals = false;
    newChunk.hasUvs = false;

    newChunk.vertOffset = vertOffset;

    if(firstOfType)
    {
        newChunk.indexCount = m_new_index_count[vtype];
        newChunk.f3Offset = m_new_f3_start[vtype];
        newChunk.f4Offset = m_new_f4_start[vtype];
    }
    /*else
    {
        MeshChunk& oldChunk = m_chunks[chunk - 1];
        if(!in_f4)
        {
            newChunk.f4Offset = oldChunk.f4Offset - new_offset;
        }
        newChunk.indexCount = oldChunk.indexCount - (new_offset - oldChunk.f3Offset);
        oldChunk.indexCount = new_offset - oldChunk.f3Offset;
        oldChunk.vertCount = (m_new_pos_count - oldChunk.vertOffset * 3) / 3;
    }*/

    if(m_vertex_maps.count(newChunk.vertOffset) == 0)
    {
        m_vertex_maps[newChunk.vertOffset] = new P3dMap<VertexIndex, uint32_t>(8192);
    }
}



void ModelLoader::copyVertDataBlender(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t>* vertexMap, const BlendData* data,
                               GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos)
{
    uint32_t new_offset = 0;
    uint32_t vert_offset = vertOffset;
    float x;
    float y;
    float z;
    uint vertCount = 0;
    P3D_LOGD("vert offset: %u", vertOffset);
    for(P3dMap<VertexIndex, uint32_t>::iterator itr = vertexMap->begin(); itr.hasNext(); ++itr)
    {
        ++vertCount;
        const VertexIndex& index = itr.key();
        uint32_t new_index = itr.value();
        P3D_LOGD("%u: %u/%u/%u > %u", vertCount, index.pos, index.uv, index.norm, new_index);
        // pos
        vert_offset = index.pos * STRIDE;
        new_offset = (new_index + vertOffset) * STRIDE;

        x = data->verts[vert_offset++];
        y = data->verts[vert_offset++];
        z = data->verts[vert_offset];

        P3D_LOGD("%u @ %u: (%f,%f,%f)", vertCount, vert_offset, x, y, z);

        if(x > m_maxX) m_maxX = x;
        if(x < m_minX) m_minX = x;
        if(y > m_maxY) m_maxY = y;
        if(y < m_minY) m_minY = y;
        if(z > m_maxZ) m_maxZ = z;
        if(z < m_minZ) m_minZ = z;
        new_pos[new_offset++] = x;
        new_pos[new_offset++] = y;
        new_pos[new_offset] = z;

        // norm
        vert_offset = index.pos * STRIDE;
        new_offset = (new_index + vertOffset) * STRIDE;
        // store empty normal
        // TODO: actual normal storage if present in BlendData
        new_norm[new_offset] = 0.0f;
        new_uv[new_offset++] = 0.0f;
        new_norm[new_offset] = 0.0f;
        new_uv[new_offset++] = 0.0f;
        new_norm[new_offset] = 0.0f;
        new_uv[new_offset] = 0.0f;
    }

    P3D_LOGD("BB: %f:%f %f:%f %f:%f", m_minX, m_maxX, m_minY, m_maxY, m_minZ, m_maxZ);
}

#include "ModelLoader.h"
#include "PlatformAdapter.h"
#include "glwrapper.h"
#include <cstring>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>


#define STRIDE 3

static P3dLogger logger("core.ModelLoader", P3dLogger::LOG_DEBUG);

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
struct P3dHash<glm::vec3>
{
    size_t operator() (const glm::vec3& k) const
    {
        size_t h1 = *((uint32_t*) &k.x);
        size_t h2 = *((uint32_t*) &k.y);
        h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        h2 = *((uint32_t*) &k.z);
        h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        return h1;
    }
};

ModelLoader::ModelLoader(IMaterialsInfo *materialsInfo)
{
    m_materialInfo = materialsInfo;
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
        for(auto item: m_gl_buffers)
        {
            glDeleteBuffers(1, &item.second->posBuffer);
            glDeleteBuffers(1, &item.second->uvBuffer);
            glDeleteBuffers(1, &item.second->normBuffer);
            delete item.second;
        }
        m_gl_buffers.clear();

        m_chunks.clear();

        glDeleteBuffers(1, &m_index_buffer);
        m_index_buffer = 0;

        for(auto item: m_vertex_maps)
        {
            delete item.second;
        }
        m_vertex_maps.clear();

        m_mat_count = 1;
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
    uint32_t chunk;
    m_mat_count = 1;

    if(chunks)
    {
        for(chunk = 0; chunk < chunkCount; ++chunk)
        {
            m_chunks.push_back(chunks[chunk]);
            if(chunks[chunk].material >= m_mat_count)
            {
                m_mat_count = chunks[chunk].material + 1;
            }
        }
    }

    generateNormals(indexBuffer, posBuffer, normBuffer, emptyNormCount);

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

    for(auto item: m_gl_buffers)
    {
        GLBuffers* glbufs = item.second;

        logger.verbose("Creating gl buf, vertOffset: %d, vertCount: %d", item.first, glbufs->vertCount);

        if(3 * (item.first + glbufs->vertCount) <= posCount)
        {
            glGenBuffers(1, &glbufs->posBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, glbufs->posBuffer);
            glBufferData(GL_ARRAY_BUFFER, 3 * glbufs->vertCount * sizeof(GLfloat),
                         posBuffer + 3 * item.first, GL_STATIC_DRAW);
        }

        if(2 * (item.first + glbufs->vertCount) <= uvCount)
        {
            glGenBuffers(1, &glbufs->uvBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, glbufs->uvBuffer);
            glBufferData(GL_ARRAY_BUFFER, 2 * glbufs->vertCount * sizeof(GLfloat),
                         uvBuffer + 2 * item.first, GL_STATIC_DRAW);
        }

        if(3 * (item.first + glbufs->vertCount) <= normCount)
        {
            glGenBuffers(1, &glbufs->normBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, glbufs->normBuffer);
            glBufferData(GL_ARRAY_BUFFER, 3 * glbufs->vertCount * sizeof(GLfloat),
                         normBuffer + 3 * item.first, GL_STATIC_DRAW);
        }
    }

    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint16_t), indexBuffer, GL_STATIC_DRAW);
    GL_CHECK_ERROR;
}

void ModelLoader::generateNormals(uint16_t *new_faces, GLfloat *new_pos, GLfloat *new_norm, uint32_t emptyNormCount)
{
    logger.debug("Generating normals");
    uint64_t start = PlatformAdapter::currentMillis();

    uint32_t i;
    uint32_t il;

    uint32_t a;
    uint32_t b;
    uint32_t c;

    uint32_t a_offset;
    uint32_t b_offset;
    uint32_t c_offset;

    P3dMap<glm::vec3, glm::vec3> normalsMap(emptyNormCount < 128
                                            ? 1
                                            : emptyNormCount / 128);

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
            if(!isnan(fnormal.x) && !isnan(fnormal.y) && !isnan(fnormal.z))
            {
                if(fnormal.x || fnormal.y || fnormal.z)
                {
                    fnormal = glm::normalize(fnormal);
                    normalsMap[posa] += fnormal;
                    normalsMap[posb] += fnormal;
                    normalsMap[posc] += fnormal;
                }
            }
        }
    }
    logger.debug("calc took: %lldms", PlatformAdapter::durationMillis(start));

    start = PlatformAdapter::currentMillis();
    // normalize
    for(auto item: normalsMap)
    {
        glm::vec3& normal = item.second;
        if(!isnan(normal.x) && !isnan(normal.y) && !isnan(normal.z))
        {
            normal = glm::normalize(normal);
        }
        else
        {
            normal = glm::vec3();
        }
    }
    logger.debug("normalize took: %lldms", PlatformAdapter::durationMillis(start));

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
    logger.debug("store took: %lldms", PlatformAdapter::durationMillis(start));

    logger.debug("Calculated %d new normals", normalsMap.size());
    normalsMap.dumpBucketLoad();
}

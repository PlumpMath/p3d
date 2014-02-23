#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "P3dVector.h"
#include "P3dMap.h"
#include "glwrapper.h"

struct MeshChunk
{
    MeshChunk()
    {
        memset(this, 0, sizeof(MeshChunk));
    }

    GLuint posBuffer;
    GLuint uvBuffer;
    GLuint normBuffer;

    uint32_t index_count[4];
    uint32_t f3_start[4];
    uint32_t f4_start[4];

    uint16_t mat;

    uint32_t vertCount;
};

class ModelLoader
{
public:
    enum VertexType {
        VT_POS_UV_NORM = 0,
        VT_POS_UV = 1,
        VT_POS_NORM = 2,
        VT_POS = 3
    };

    ModelLoader();
    virtual ~ModelLoader();
    bool load(const char* data, size_t size);
    bool isLoaded() { return m_loaded; }
    void clear();
    int chunkCount() { return m_chunks.size(); }
    GLuint posBuffer(uint32_t chunk) { return m_chunks[chunk].posBuffer; }
    GLuint uvBuffer(uint32_t chunk) { return m_chunks[chunk].uvBuffer; }
    GLuint normBuffer(uint32_t chunk) { return m_chunks[chunk].normBuffer; }
    GLuint indexBuffer() { return m_index_buffer; }
    uint32_t indexCount(uint32_t chunk, VertexType vtype) { return m_chunks[chunk].index_count[vtype]; }
    uint32_t indexOffset(uint32_t chunk, VertexType vtype) { return m_chunks[chunk].f3_start[vtype]; }
    float boundingRadius();

    void copyVertData(uint32_t chunk, const char* data, GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos);
private:
    struct VertexIndex
    {
        uint32_t pos;
        uint32_t uv;
        uint32_t norm;
        ModelLoader::VertexType type;
        bool operator==(const VertexIndex &other) const;
        size_t hash() const;
    };

    size_t addPadding(size_t size);
    bool reindex(const char *data);
    uint32_t reindexType(uint32_t &chunk, VertexType vtype, const char* data,
                         uint16_t *new_faces, uint16_t *new_mats);
    void generateNormals(uint32_t chunk, uint16_t *new_faces, GLfloat* new_pos, GLfloat* new_norm);

    bool m_loaded;

    uint32_t m_pos_count;
    uint32_t m_norm_count;
    uint32_t m_tex_count;

    uint32_t m_f3_count[4];
    uint32_t m_f4_count[4];

    uint32_t m_pos_start;
    uint32_t m_norm_start;
    uint32_t m_tex_start;

    uint32_t m_f3_start[4];
    uint32_t m_f4_start[4];

    uint16_t m_mat_count;

    // new data
    P3dVector<MeshChunk> m_chunks;

    P3dVector<P3dMap<VertexIndex, uint32_t>*> m_vertex_maps;

    P3dVector<uint32_t> m_new_pos_offsets;
    P3dVector<uint32_t> m_new_norm_offsets;
    P3dVector<uint32_t> m_new_uv_offsets;

    size_t m_total_index_count;

    uint32_t m_new_pos_count;
    uint32_t m_new_norm_count;
    uint32_t m_new_uv_count;

    // OpenGL
    GLuint m_index_buffer;

    // bounding box
    float m_maxX;
    float m_minX;
    float m_maxY;
    float m_minY;
    float m_maxZ;
    float m_minZ;


};

#endif // MODELLOADER_H

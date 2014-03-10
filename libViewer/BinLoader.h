#ifndef BINLOADER_H
#define BINLOADER_H

#include "BaseLoader.h"

#include <cstdlib>
#include <cstdint>

#include "P3dMap.h"
#include "glwrapper.h"

class BinLoader : public BaseLoader
{
public:
    BinLoader();
    virtual ~BinLoader();

    bool load(const char* data, size_t size);
private:
    size_t addPadding(size_t size);

    bool reindex(const char *data);
    uint32_t reindexType(uint32_t &chunk, VertexType vtype, const char* data,
                         uint16_t *new_faces, uint16_t *new_mats);
    void copyVertData(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t>* vertexMap, const char* data,
                      GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos);
    void nextChunk(uint32_t &chunk, VertexType vtype, bool in_f4, uint32_t new_offset,
                   uint32_t vertOffset, bool firstOfType = false);

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

    // bounding box
    float m_maxX;
    float m_minX;
    float m_maxY;
    float m_minY;
    float m_maxZ;
    float m_minZ;

    // new data
    P3dVector<MeshChunk> m_chunks;

    P3dMap<uint32_t, P3dMap<VertexIndex, uint32_t>*> m_vertex_maps;

    uint32_t m_new_index_count[4];
    uint32_t m_new_f3_start[4];
    uint32_t m_new_f4_start[4];

    size_t m_total_index_count;

    uint32_t m_new_pos_count;
    uint32_t m_new_norm_count;
    uint32_t m_new_empty_norm_count;
    uint32_t m_new_uv_count;

};

#endif // BINLOADER_H

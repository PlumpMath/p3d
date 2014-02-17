#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <cstdlib>
#include <cstdint>
#include "P3dVector.h"
#include "P3dMap.h"
#include "glwrapper.h"

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
    int chunkCount() { return m_pos_buffer_id.size(); }
    GLuint posBuffer(int chunk) { return m_pos_buffer_id[chunk]; }
    GLuint uvBuffer(int chunk) { return m_uv_buffer_id[chunk]; }
    GLuint normBuffer(int chunk) { return m_norm_buffer_id[chunk]; }
    GLuint indexBuffer(int chunk) { return m_index_buffer_id[chunk]; }
    uint32_t indexCount(int chunk, VertexType vtype) { return m_index_count[chunk][vtype]; }
    uint32_t indexOffset(int chunk, VertexType vtype) { return m_new_f3_start[chunk][vtype]; }
    float boundingRadius();

    void copyVertData(const char* data, GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos);
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
    bool deindex(const char *data);
    uint32_t deindexType(int &chunk, VertexType vtype, const char* data,
                         uint16_t *new_faces, uint16_t *new_mats);
    void generateNormals(int chunk, uint16_t *new_faces, GLfloat* new_pos, GLfloat* new_norm);

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
    P3dMap<VertexIndex, uint32_t> m_vertex_map;

    size_t m_total_index_count;
    P3dVector<uint32_t[4]> m_index_count;
    P3dVector<uint32_t[4]> m_new_f3_start;
    P3dVector<uint32_t[4]> m_new_f4_start;

    uint32_t m_new_pos_count;
    uint32_t m_new_norm_count;
    uint32_t m_new_uv_count;

    // OpenGL
    P3dVector<GLuint> m_pos_buffer_id;
    P3dVector<GLuint> m_uv_buffer_id;
    P3dVector<GLuint> m_norm_buffer_id;
    P3dVector<GLuint> m_index_buffer_id;

    // bounding box
    float m_maxX;
    float m_minX;
    float m_maxY;
    float m_minY;
    float m_maxZ;
    float m_minZ;


};

#endif // MODELLOADER_H

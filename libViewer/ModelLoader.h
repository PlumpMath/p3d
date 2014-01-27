#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <cstdlib>
#include <cstdint>

typedef unsigned int GLuint;
typedef unsigned int GLenum;

class ModelLoader
{
public:
    ModelLoader();
    virtual ~ModelLoader();
    bool load(const char* data, size_t size);
    bool isLoaded() { return m_loaded; }
    GLuint posBuffer() { return m_pos_buffer_id; }
    GLuint uvBuffer() { return m_uv_buffer_id; }
    GLuint normBuffer() { return m_norm_buffer_id; }
    GLuint indexBuffer() { return m_index_buffer_id; }
    uint32_t indexCount() { return m_index_count_pos_uv_norm; }
    float boundingRadius();

private:
    size_t addPadding(size_t size);
    void deindex(const char *data);

    bool m_loaded;
    const char* m_data;

    uint32_t m_pos_count;
    uint32_t m_norm_count;
    uint32_t m_tex_count;

    uint32_t m_f3_count_pos;
    uint32_t m_f3_count_pos_norm;
    uint32_t m_f3_count_pos_uv;
    uint32_t m_f3_count_pos_uv_norm;

    uint32_t m_f4_count_pos;
    uint32_t m_f4_count_pos_norm;
    uint32_t m_f4_count_pos_uv;
    uint32_t m_f4_count_pos_uv_norm;

    uint32_t m_index_count_pos_uv_norm;

    uint32_t m_pos_start;
    uint32_t m_norm_start;
    uint32_t m_tex_start;

    uint32_t m_f3_start_pos;
    uint32_t m_f3_start_pos_norm;
    uint32_t m_f3_start_pos_uv;
    uint32_t m_f3_start_pos_uv_norm;

    uint32_t m_f4_start_pos;
    uint32_t m_f4_start_pos_norm;
    uint32_t m_f4_start_pos_uv;
    uint32_t m_f4_start_pos_uv_norm;

    uint16_t m_mat_count;

    // OpenGL
    GLuint m_pos_buffer_id;
    GLuint m_uv_buffer_id;
    GLuint m_norm_buffer_id;
    GLuint m_index_buffer_id;

    // bounding box
    float m_maxX;
    float m_minX;
    float m_maxY;
    float m_minY;
    float m_maxZ;
    float m_minZ;


};

#endif // MODELLOADER_H

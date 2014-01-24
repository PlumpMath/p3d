#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <cstdlib>
#include <cstdint>

class ModelLoader
{
public:
    ModelLoader();
    virtual ~ModelLoader();
    bool load(const char* data, size_t size);
private:
    size_t addPadding(size_t size);
    void deindex(const char *data);

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
};

#endif // MODELLOADER_H

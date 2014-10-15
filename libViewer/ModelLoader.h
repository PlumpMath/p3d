#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "P3dVector.h"
#include "P3dMap.h"
#include "glwrapper.h"

#include "BaseLoader.h"

struct GLBuffers;
class IMaterialsInfo;

class ModelLoader
{
public:
    enum VertexType {
        VT_POS_UV_NORM = 0,
        VT_POS_UV = 1,
        VT_POS_NORM = 2,
        VT_POS = 3
    };

    explicit ModelLoader(IMaterialsInfo* materialsInfo = nullptr);
    virtual ~ModelLoader();

    //accessors
    IMaterialsInfo* materialsInfo() {return m_materialInfo;}

    bool isLoaded() { return m_loaded; }
    void setIsLoaded(bool newValue) { m_loaded = newValue; }
    void clear();
    int chunkCount() { return m_chunks.size(); }
    GLuint posBuffer(uint32_t chunk);
    GLuint uvBuffer(uint32_t chunk);
    GLuint normBuffer(uint32_t chunk);
    GLuint indexBuffer() { return m_index_buffer; }
    uint32_t indexCount(uint32_t chunk) { return m_chunks[chunk].indexCount; }
    uint32_t indexOffset(uint32_t chunk) { return m_chunks[chunk].f3Offset; }
    uint16_t material(uint32_t chunk) { return m_chunks[chunk].material; }
    uint16_t materialCount() { return m_mat_count; }
    bool hasUvs(uint32_t chunk) { return m_chunks[chunk].hasUvs; }
    float boundingRadius();
    void setBoundingBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
    void createModel(uint32_t posCount, uint32_t normCount, uint32_t emptyNormCount, uint32_t uvCount,
                     float* posBuffer, float* normBuffer, float* uvBuffer, uint32_t indexCount,
                     uint16_t* indexBuffer, uint32_t chunkCount, const MeshChunk *chunks);
private:
    IMaterialsInfo* m_materialInfo = nullptr;

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
    void generateNormals(uint16_t *new_faces, GLfloat* new_pos, GLfloat* new_norm, uint32_t emptyNormCount);

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

    uint16_t m_mat_count = 0;

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

    // OpenGL
    GLuint m_index_buffer;
    P3dMap<uint32_t, GLBuffers*> m_gl_buffers;

    // bounding box
    float m_maxX;
    float m_minX;
    float m_maxY;
    float m_minY;
    float m_maxZ;
    float m_minZ;


};

#endif // MODELLOADER_H

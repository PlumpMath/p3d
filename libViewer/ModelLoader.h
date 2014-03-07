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

    uint32_t vertCount;
    uint32_t vertOffset;

    bool validNormals;
    bool hasUvs;

    uint32_t indexCount;
    uint32_t f3Offset;
    uint32_t f4Offset;

    uint16_t material;
};

#define STRIDE 3
/** Class to pass around data buffers from Blender files */
class BlendData {
public:
    BlendData()
    {
        totvert = 0;
        totface = 0;
        vertbytes = 0;
        facebytes = 0;
        verts = 0;
        faces = 0;
        isloaded = false;
    }
    ~BlendData()
    {
        clearBlendData();
    }

    /** Data is expected to be triangulated before being passed in here. */
    void initBlendData(uint32_t tv, uint32_t tf, float *vs, uint32_t *fs) {
        isloaded = false;
        if(verts != 0) delete [] verts;
        if(faces != 0) delete [] faces;

        totvert = tv;
        totface = tf;

        vertbytes = totvert * sizeof(float) * STRIDE;
        facebytes = totface * sizeof(uint32_t) * STRIDE;

        verts = new float[totvert*STRIDE];
        faces = new uint32_t[totface*STRIDE];

        float *v, *vnew;
        uint32_t *f, *fnew;
        uint i;
        for(i=0, v = vs, vnew = verts; i < tv*STRIDE; i++, v++, vnew++) {
            *vnew = *v;
            P3D_LOGD("v %f\n", *vnew);
        }

        for(i=0, f = fs, fnew = faces; i < tf*STRIDE; i++, f++, fnew++) {
            *fnew = *f;
            P3D_LOGD("f %u\n", *fnew);
        }

        if(totvert>0 && verts!=NULL) {
            isloaded = true;
        } else {
            clearBlendData();
        }
    }

    void clearBlendData() {
        isloaded = false;
        delete [] verts;
        delete [] faces;
        verts = 0;
        faces = 0;
        totvert = 0;
        totface = 0;
        vertbytes = 0;
        facebytes = 0;
    }

    size_t size() const { return vertbytes + facebytes; }

    bool isLoaded() { return isloaded; }

    uint32_t totvert;
    float *verts;
    size_t vertbytes;

    uint32_t totface;
    uint32_t *faces;
    size_t facebytes;

    bool isloaded;
};

struct GLBuffers;

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
    bool load(const BlendData *blendData); // << load blender data
    bool isLoaded() { return m_loaded; }
    void clear();
    int chunkCount() { return m_chunks.size(); }
    GLuint posBuffer(uint32_t chunk);
    GLuint uvBuffer(uint32_t chunk);
    GLuint normBuffer(uint32_t chunk);
    GLuint indexBuffer() { return m_index_buffer; }
    uint32_t indexCount(uint32_t chunk) { return m_chunks[chunk].indexCount; }
    uint32_t indexOffset(uint32_t chunk) { return m_chunks[chunk].f3Offset; }
    uint16_t material(uint32_t chunk) { return m_chunks[chunk].material; }
    bool hasUvs(uint32_t chunk) { return m_chunks[chunk].hasUvs; }
    float boundingRadius();
    void createModel(uint32_t posCount, uint32_t normCount, uint32_t uvCount,
                     float* posBuffer, float* normBuffer, float* uvBuffer, uint32_t indexCount,
                     uint16_t* indexBuffer, uint32_t chunkCount, MeshChunk* chunks);

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
    void generateNormals(uint16_t *new_faces, GLfloat* new_pos, GLfloat* new_norm);
    void copyVertData(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t>* vertexMap, const char* data,
                      GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos);
    void nextChunk(uint32_t &chunk, ModelLoader::VertexType vtype, bool in_f4, uint32_t new_offset,
                   uint32_t vertOffset, bool firstOfType = false);

    uint32_t reindexTypeBlender(uint32_t &chunk, VertexType vtype, const BlendData *blendData,
                         uint16_t *new_faces); // << reindex blender data for specified vertex type
    void copyVertDataBlender(uint32_t vertOffset, P3dMap<VertexIndex, uint32_t>* vertexMap, const BlendData* data,
                      GLfloat* new_norm, GLfloat* new_uv, GLfloat* new_pos); // << copy blender vertex data into GLfloat buffers
    void nextChunkBlender(uint32_t &chunk, ModelLoader::VertexType vtype, bool in_f4, uint32_t new_offset,
                   uint32_t vertOffset, bool firstOfType = false); // << create a new chunk, blender specific

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

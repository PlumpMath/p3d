#ifndef BASELOADER_H
#define BASELOADER_H

#include <cstdint>
#include <cstring>

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

class ModelLoader;

//! \brief base class for loaders
class BaseLoader
{
public:
    enum VertexType {
        VT_POS_UV_NORM = 0,
        VT_POS_UV = 1,
        VT_POS_NORM = 2,
        VT_POS = 3
    };

    BaseLoader();
    virtual ~BaseLoader() {}
    void setModelLoader(ModelLoader* modelLoader) { m_modelLoader = modelLoader; }
    static BaseLoader* loaderFromExtension(const char* extension);
    virtual bool load(const char*, size_t) { return false; }

protected:
    struct VertexIndex
    {
        uint32_t pos;
        uint32_t uv;
        uint32_t norm;
        BaseLoader::VertexType type;
        bool operator==(const VertexIndex &other) const;
        size_t hash() const;
    };

    ModelLoader* m_modelLoader;
};

class RegisterLoader
{
public:
    //! \brief registers a loader supporting a list of extensions
    //! \arg loader pointer to the laoder to register
    //! \arg ... zero terminated list of exensions
    //! \example ReisterLoader registerMyLoader(&myLoader, ".foo", ".bar", 0);
    RegisterLoader(BaseLoader* loader, ...);
};

#endif // BASELOADER_H

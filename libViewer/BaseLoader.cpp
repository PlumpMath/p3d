#include "BaseLoader.h"
#include "PlatformAdapter.h"
#include "P3dMap.h"

#include <stdarg.h>

static P3dMap<const char*, BaseLoader*> loaderRegistry(16);

bool BaseLoader::VertexIndex::operator==(const VertexIndex &other) const
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

size_t BaseLoader::VertexIndex::hash() const
{
    size_t h1 = pos;
    size_t h2 = uv;
    h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
    h2 = norm;
    h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
    return h1;
}

BaseLoader::BaseLoader()
{
    m_modelLoader = 0;
}

BaseLoader *BaseLoader::loaderFromExtension(const char *extension)
{
    if(loaderRegistry.count(extension))
    {
        return loaderRegistry[extension];
    }
    return 0;
}


RegisterLoader::RegisterLoader(BaseLoader *loader, ...)
{
    va_list args;
    va_start(args, loader);

    const char* ext;
    while(true)
    {
        ext = va_arg(args, const char*);
        if(!ext || ext[0] != '.')
        {
            break;
        }
        loaderRegistry.insert(ext, loader);
    }
    va_end(args);
}

#ifndef PLATFORMADAPTER_H
#define PLATFORMADAPTER_H

#include <cstdlib>

class PlatformAdapter
{
public:
    PlatformAdapter();
    virtual ~PlatformAdapter();

    //! \brief load asset data, e.g. shader code
    //! \arg filename path of asset, e.g. shaders/fragment.glsl
    //! \arg size pointer to receive size of data or 0 to get zero terminated data (default)
    //! \return loaded data or 0 on error. Caller is responsible for freeing data
    virtual const char* loadAsset(const char* filename, size_t *size = 0);
};

#endif // PLATFORMADAPTER_H

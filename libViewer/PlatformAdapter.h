#ifndef PLATFORMADAPTER_H
#define PLATFORMADAPTER_H

#include <cstdlib>

#define  P3D_LOGD(...) PlatformAdapter::adapter->logFunc(__PRETTY_FUNCTION__, __VA_ARGS__)
#define  P3D_LOGE(...) PlatformAdapter::adapter->logFunc(__PRETTY_FUNCTION__, __VA_ARGS__)

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

    //! \brief writes out a printf formattet log messages
    virtual void logFunc(const char* func, const char* format, ...);

    //! \brief the PlatformAdapter instance
    //! \note this can't be done as normal singleton because it is initialized from user of the lib
    static PlatformAdapter* adapter;
};

#endif // PLATFORMADAPTER_H

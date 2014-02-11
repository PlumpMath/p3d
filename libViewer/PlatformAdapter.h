#ifndef PLATFORMADAPTER_H
#define PLATFORMADAPTER_H

#include <cstdlib>
#include <stdarg.h>

#define  P3D_LOGD(...) PlatformAdapter::adapter->logFunc(PlatformAdapter::LOG_DEBUG, __PRETTY_FUNCTION__, __VA_ARGS__)
#define  P3D_LOGE(...) PlatformAdapter::adapter->logFunc(PlatformAdapter::LOG_ERROR, __PRETTY_FUNCTION__, __VA_ARGS__)

#define GL_CHECK_ERROR {GLenum err = glGetError(); if(err != GL_NO_ERROR) P3D_LOGE("%s:%d ogl error: 0x%x", __FILE__, __LINE__, err);}

class PlatformAdapter
{
public:
	enum LogLevel {
		LOG_DEBUG = 600,
		LOG_INFO = 400,
		LOG_WARN = 300,
		LOG_ERROR = 200,
		LOG_FATAL = 100
	};
    PlatformAdapter();
    virtual ~PlatformAdapter();

    //! \brief load asset data, e.g. shader code
    //! \arg filename path of asset, e.g. shaders/fragment.glsl
    //! \arg size pointer to receive size of data or 0 to get zero terminated data (default)
    //! \return loaded data or 0 on error. Caller is responsible for freeing data
    virtual const char* loadAsset(const char* filename, size_t *size = 0);

    //! \brief writes out a printf formattet log messages
    //! \arg level severity level
    //! \arg func pretty function info of caller (__PRETTY_FUNCTION__)
    //! \arg format msg format, like printf
    virtual void logFunc(LogLevel level, const char* func, const char* format, ...);

    virtual void logTag(LogLevel level, const char* tag, const char* format, va_list args);

    //! \brief the PlatformAdapter instance
    //! \note this can't be done as normal singleton because it is initialized from user of the lib
    static PlatformAdapter* adapter;
};

#endif // PLATFORMADAPTER_H

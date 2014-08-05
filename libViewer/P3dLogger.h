#ifndef P3DLOGGER_H
#define P3DLOGGER_H

#define  P3D_LOGV(...) PlatformAdapter::adapter->logFunc(P3dLogger::LOG_VERBOSE, __PRETTY_FUNCTION__, __VA_ARGS__)
#define  P3D_LOGD(...) PlatformAdapter::adapter->logFunc(P3dLogger::LOG_DEBUG, __PRETTY_FUNCTION__, __VA_ARGS__)
#define  P3D_LOGI(...) PlatformAdapter::adapter->logFunc(P3dLogger::LOG_INFO, __PRETTY_FUNCTION__, __VA_ARGS__)
#define  P3D_LOGW(...) PlatformAdapter::adapter->logFunc(P3dLogger::LOG_WARN, __PRETTY_FUNCTION__, __VA_ARGS__)
#define  P3D_LOGE(...) PlatformAdapter::adapter->logFunc(P3dLogger::LOG_ERROR, __PRETTY_FUNCTION__, __VA_ARGS__)
#define  P3D_LOGF(...) PlatformAdapter::adapter->logFunc(P3dLogger::LOG_FATAL, __PRETTY_FUNCTION__, __VA_ARGS__)

class P3dLogger
{
public:
    enum Level {
        LOG_VERBOSE = 1000,
        LOG_DEBUG = 600,
        LOG_INFO = 400,
        LOG_WARN = 300,
        LOG_ERROR = 200,
        LOG_FATAL = 100
    };
    P3dLogger(const char* tag, Level level=LOG_VERBOSE);
    void verbose(const char* format, ...) const;
    void debug(const char* format, ...) const;
    void info(const char* format, ...) const;
    void warning(const char* format, ...) const;
    void error(const char* format, ...) const;
    void fatal(const char* format, ...) const;

private:
    const char* m_tag;
    Level m_level;
};

#endif // P3DLOGGER_H

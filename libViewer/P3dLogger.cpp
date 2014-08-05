#include "P3dLogger.h"
#include "PlatformAdapter.h"

#include <stdarg.h>

void P3dLogger::verbose(const char *format, ...) const
{
    if(m_level < LOG_VERBOSE) return;
    va_list args;
    va_start(args, format);
    PlatformAdapter::adapter->logTag(LOG_VERBOSE, m_tag, format, args);
    va_end(args);
}

void P3dLogger::debug(const char *format, ...) const
{
    if(m_level < LOG_DEBUG) return;
    va_list args;
    va_start(args, format);
    PlatformAdapter::adapter->logTag(LOG_DEBUG, m_tag, format, args);
    va_end(args);
}

void P3dLogger::info(const char *format, ...) const
{
    if(m_level < LOG_INFO) return;
    va_list args;
    va_start(args, format);
    PlatformAdapter::adapter->logTag(LOG_INFO, m_tag, format, args);
    va_end(args);
}

void P3dLogger::warning(const char *format, ...) const
{
    if(m_level < LOG_WARN) return;
    va_list args;
    va_start(args, format);
    PlatformAdapter::adapter->logTag(LOG_WARN, m_tag, format, args);
    va_end(args);
}

void P3dLogger::error(const char *format, ...) const
{
    if(m_level < LOG_ERROR) return;
    va_list args;
    va_start(args, format);
    PlatformAdapter::adapter->logTag(LOG_ERROR, m_tag, format, args);
    va_end(args);
}

void P3dLogger::fatal(const char *format, ...) const
{
    if(m_level < LOG_FATAL) return;
    va_list args;
    va_start(args, format);
    PlatformAdapter::adapter->logTag(LOG_FATAL, m_tag, format, args);
    va_end(args);
}

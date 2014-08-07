#include "PlatformAdapter.h"
#include <cstdio>
#include <cstring>

#include <sys/time.h>

PlatformAdapter* PlatformAdapter::adapter = 0;

static P3dLogger logger("core.PlatformAdapter", P3dLogger::LOG_DEBUG);

PlatformAdapter::PlatformAdapter()
{
}

PlatformAdapter::~PlatformAdapter()
{
}

void PlatformAdapter::loadTexture(const char *name, std::function<void(uint32_t)> callback)
{
    (void) name;
    (void) callback;
}

void PlatformAdapter::deleteTexture(uint32_t textureId)
{
    (void) textureId;
}

void PlatformAdapter::cancelTextureLoads()
{
}

const char *PlatformAdapter::loadAsset(const char *filename, size_t *size)
{
    size_t filesize;
    FILE* f = fopen(filename, "rb");
    if(!f) {
        logger.error("Unable to load asset: %s", filename);
        return 0;
    }
    fseek(f, 0L, SEEK_END);
    filesize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *data;
    if(size)
    {
        *size = filesize;
        data = new char[filesize];
    }
    else
    {
        data = new char[filesize + 1];
    }
    fread(data, filesize, 1, f);
    fclose(f);
    if(!size)
    {
        data[filesize] = 0;
    }
    return data;
}

void PlatformAdapter::logFunc(P3dLogger::Level level, const char *func, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // try to extract tag (c++ class)
    char* tag_buf = new char[strlen(func) + 1];
    char* tag = tag_buf;
    strcpy(tag, func);
    char* pos = strstr(tag, "::");
    if(pos)
    {
        *pos = 0;
        pos = strchr(tag, ' ');
        if(pos)
        {
            tag = ++pos;
        }
    }

    logTag(level, tag, format, args);

    va_end(args);
    delete [] tag_buf;
}

void PlatformAdapter::logTag(P3dLogger::Level level, const char *tag, const char *format, va_list args)
{
    FILE* out = stdout;

    switch(level)
    {
    case P3dLogger::Level::LOG_VERBOSE:
        fprintf(out, "V ");
        break;
    case P3dLogger::Level::LOG_DEBUG:
        fprintf(out, "D ");
        break;
    case P3dLogger::Level::LOG_INFO:
        fprintf(out, "I ");
        break;
    case P3dLogger::Level::LOG_WARN:
        out = stderr;
        fprintf(out, "W ");
        break;
    case P3dLogger::Level::LOG_ERROR:
        out = stderr;
        fprintf(out, "E ");
        break;
    case P3dLogger::Level::LOG_FATAL:
        out = stderr;
        fprintf(out, "F ");
        break;
    default:
        fprintf(out, "U ");
        break;
    }

    fprintf(out, "%s: ", tag);
    vfprintf(out, format, args);
    fprintf(out, "\n");
    fflush(out);
}

uint64_t PlatformAdapter::currentMillis()
{
    return adapter->_currentMillis();
}

uint64_t PlatformAdapter::durationMillis(uint64_t timestamp)
{
    return adapter->_currentMillis() - timestamp;
}

uint64_t PlatformAdapter::_currentMillis()
{
    struct timeval res;
    gettimeofday(&res, NULL);
    return res.tv_sec * 1000 + res.tv_usec / 1000;
}

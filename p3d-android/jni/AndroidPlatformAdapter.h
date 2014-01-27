#ifndef ANDROIDPLATFORMADAPTER_H_
#define ANDROIDPLATFORMADAPTER_H_

#include "PlatformAdapter.h"

class AAssetManager;

class AndroidPlatformAdapter : public PlatformAdapter {
public:
	AndroidPlatformAdapter();
	virtual ~AndroidPlatformAdapter();
	virtual const char* loadAsset(const char* filename, size_t *size = 0);
	virtual void logTag(LogLevel level, const char* tag, const char* format, va_list args);

	void setAssetManager(AAssetManager* assetManager);

private:
	AAssetManager* m_AssetManager;
};

#endif /* ANDROIDPLATFORMADAPTER_H_ */

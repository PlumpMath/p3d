/*
 * AndroidPlatformAdapter.cpp
 *
 *  Created on: Jan 21, 2014
 *      Author: pelle
 */

#include "AndroidPlatformAdapter.h"
#include <android/asset_manager.h>
#include <android/log.h>

#define  LOG_TAG    "AndroidPlatformAdapter"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


AndroidPlatformAdapter::AndroidPlatformAdapter() {
	m_AssetManager = 0;
}

AndroidPlatformAdapter::~AndroidPlatformAdapter() {
	// TODO Auto-generated destructor stub
}

const char* AndroidPlatformAdapter::loadAsset(const char* filename,
		size_t* size) {
	LOGD("loadAsset %s", filename);

	AAsset* asset = AAssetManager_open(m_AssetManager, filename, AASSET_MODE_UNKNOWN);
	if (NULL == asset) {
		LOGE("_ASSET_NOT_FOUND_");
		return 0;
	}
	size_t filesize = AAsset_getLength(asset);
	char* data;
	if(size) {
		*size = filesize;
		data = new char[filesize];
	} else {
		data = new char[filesize + 1];
	}

	AAsset_read (asset,data,filesize);
	LOGD("asset %s: %s", filename, data);
	AAsset_close(asset);

	if(!size) {
		data[filesize] = 0;
	}

	return data;
}

void AndroidPlatformAdapter::setAssetManager(AAssetManager* assetManager) {
	m_AssetManager = assetManager;
}

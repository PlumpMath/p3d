#include "jni_stub.h"
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "P3dViewer.h"
#include "CameraNavigation.h"
#include "AndroidPlatformAdapter.h"

#define  LOG_TAG    "jni"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

AndroidPlatformAdapter* adapter = new AndroidPlatformAdapter();
P3dViewer viewer(adapter);
static int _width = 0;
static int _height = 0;

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_on_1surface_1created
  (JNIEnv *env, jclass cls)
{
	// unused
	(void)env;
	(void)cls;

	LOGD("surfaceCreated");
	viewer.onSurfaceCreated();
}

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_on_1surface_1changed
  (JNIEnv *env, jclass cls, jint width, jint height)
{
	// unused
	(void)env;
	(void)cls;

	_width = width;
	_height = height;
	viewer.onSurfaceChanged(_width, _height);
}

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_on_1draw_1frame
  (JNIEnv *env, jclass cls)
{
	// unused
	(void)env;
	(void)cls;

	viewer.drawFrame();
}

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_init_1asset_1manager
  (JNIEnv *env, jclass cls, jobject am)
{
	// unused
	(void)cls;

	LOGD("assetmgr");
	AAssetManager* mgr = AAssetManager_fromJava(env, am);
	adapter->setAssetManager(mgr);
}

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_load_1model(JNIEnv* env,
		jclass cls, jobject buf, jint size, jstring extension) {
	// unused
	(void)cls;

	const char* data = (const char*) env->GetDirectBufferAddress(buf);
	const char *c_extension = env->GetStringUTFChars(extension, 0);
	viewer.loadModel(data, size, c_extension);
	env->ReleaseStringUTFChars(extension, c_extension);
}

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_start_1rotate_1cam
  (JNIEnv *env, jclass cls, jfloat x, jfloat y) {
	// unused
	(void)env;
	(void)cls;
	viewer.cameraNavigation()->startRotate(x, y);
}

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_rotate_1cam
  (JNIEnv *env, jclass cls, jfloat x, jfloat y) {
	// unused
	(void)env;
	(void)cls;
	viewer.cameraNavigation()->rotate(x, y);
}

JNIEXPORT void JNICALL Java_in_p3d_mobile_P3dViewerJNIWrapper_reset_1cam
  (JNIEnv *env, jclass cls) {
	// unused
	(void)env;
	(void)cls;
	viewer.cameraNavigation()->reset();
}

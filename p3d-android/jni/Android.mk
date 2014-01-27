LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := p3dviewer
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../libViewer $(LOCAL_PATH)/../../ext/glm
LOCAL_CFLAGS    := -Wall -Wextra -std=c++0x
LOCAL_SRC_FILES := \
	../../libViewer/PlatformAdapter.cpp \
	../../libViewer/P3dViewer.cpp \
	../../libViewer/ModelLoader.cpp \
	jni_stub.cpp \
	AndroidPlatformAdapter.cpp
LOCAL_LDLIBS	:= -lGLESv2 -llog -landroid

include $(BUILD_SHARED_LIBRARY)

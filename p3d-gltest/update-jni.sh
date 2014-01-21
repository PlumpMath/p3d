#!/bin/sh
javah -classpath $HOME/android-sdk-linux/platforms/android-14/android.jar:bin/classes -o jni/jni_stub.h in.p3d.gltest.P3dViewerJNIWrapper

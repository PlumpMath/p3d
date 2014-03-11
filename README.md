p3d.in c++ viewer
=================

This is using git submodules for external libs (glm), so:

    $git submodule init
    $git submodule update

Building with QtCreator
=======================

Open three projects in the same session:

 - p3d.pro
 - zlib.pro
 - p3dconverter.pro

For all the base build directory should be ./build/debug and ./build/release
for the respective build types. For zlib and p3dconverter, append /zlib and
/p3dconverter respectively. This ensures all internally built libraries can be
found and linked.

Set p3d.pro as the active project.

Set p3dconverter.pro as dependency for p3d.pro.

Set zlib.pro as dependency for p3dconverter.pro.

Structure
=========

Contains following projects:

 - libViewer: the viewer library
 - p3d-qt: Qt app using libViewer
 - p3d-android: Android NDK app using libViewer
 - p3d-em: Emscripten test using libViewer

p3d.in c++ viewer
=================

This is using git submodules for external libs (glm, P3dConverter), so:

    $git submodule init
    $git submodule update

Building with P3dConverter
==========================

Initialize and update submodules as per first section.

In QtCreator with p3d project open add a new project. Select the CMakeLists.txt
file from the P3dConverter directory.

As build directory make sure you use ./build/p3dconverter (seen from README.md).

In project settings for P3dConverter go to Build and Run settings. From that
choose Build. Set the Build Steps for P3dConverter to:

   make zlibstatic fbtFile bfBlend p3dConvert

Set this new project to be the dependency for the p3d project.

Structure
=========

Contains following projects:

 - libViewer: the viewer library
 - p3d-qt: Qt app using libViewer
 - p3d-android: Android NDK app using libViewer
 - p3d-em: Emscripten test using libViewer

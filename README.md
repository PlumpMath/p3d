p3d.in c++ viewer
=================

This is using git submodules for external libs (glm), so:

    $git submodule init
    $git submodule update

Building with QtCreator
=======================

p3d-qt
------
Open p3d-qt/p3d.pro and build

p3d-em
------
 - Open p3d-em/p3d-em.pro
 - Build options > Build steps > Make > Override /usr/bin/make: emmake
 - Build options > Build steps > Make > Make arguments: make
 - Run options > Run > Add > Custom
   - Executable: xdg-open
   - Arguments: em_test.html
   - Working directory: %{buildDir}

Run chrome with: --allow-file-access-from-files for emscripten to be able to load data files locally

Structure
=========

Contains following projects:

 - libViewer: the viewer library
 - p3d-qt: Qt app using libViewer
 - p3d-android: Android NDK app using libViewer
 - p3d-em: Emscripten test using libViewer

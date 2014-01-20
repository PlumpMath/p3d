#-------------------------------------------------
#
# Project created by QtCreator 2014-01-20T11:18:33
#
#-------------------------------------------------

QT       -= core gui

TARGET = libViewer
TEMPLATE = lib
CONFIG += staticlib

SOURCES += P3dViewer.cpp

HEADERS += P3dViewer.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

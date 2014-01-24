#!/bin/sh
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
echo $SCRIPTPATH
ln -sf $SCRIPTPATH/../libViewer/shaders assets/shaders

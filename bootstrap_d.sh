#!/bin/bash

export PREFIX=$HOME/devroot

export SCRIPT_DIR=`dirname $0`;
export CMAKE_PREFIX_PATH=$PREFIX:$CMAKE_PREFIX_PATH
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig/:$PKG_CONFIG_PATH
cmake \
    -G "Eclipse CDT4 - Unix Makefiles" \
    -DCMAKE_INSTALL_PREFIX=$PREFIX $SCRIPT_DIR \
    -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" \
    -DCMAKE_MODULE_PATH="$PREFIX/share/cmake-2.8/Modules" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-j6"


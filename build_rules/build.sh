#!/bin/bash

set -e

BUILD_PATH=`pwd`
DEBUG_FLAGS="-g -O0 -DDEBUG"
RELEASE_FLAGS="-fPIC -O2 -DDEBUG"
FINAL_FLAGS="-fPIC -O2"
EXTAR_CXXFLAGS=$FINAL_FLAGS

# debug/release
if [ $# -gt 0 ]; then
    if [ "$1" == "--debug" ]; then
        EXTAR_CXXFLAGS=$DEBUG_FLAGS
    elif [ "$1" == "--release" ]; then
        EXTAR_CXXFLAGS=$RELEASE_FLAGS
    fi
    shift
fi

cd ../src
./configure --extra-cxxflags="$EXTAR_CXXFLAGS -std=c++11 -frtti -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections"
make clean
make -j4
cd $BUILD_PATH

bash test.sh
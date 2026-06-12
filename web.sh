#!/bin/bash
set -e

export PATH=$PATH:/usr/lib/emscripten

BUILD_DIR="build-web"

cd $BUILD_DIR

emcmake cmake ..
emmake make -j$(nproc)

echo "built"
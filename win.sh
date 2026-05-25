#!/bin/bash
set -e

BUILD_DIR="build-windows"

if [ "$1" = "clean" ] || [ -d "$BUILD_DIR" ]; then
    echo "removing old build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if command -v x86_64-w64-mingw32-cmake &> /dev/null; then
    CMAKE_CMD="x86_64-w64-mingw32-cmake"
    echo "using $CMAKE_CMD (native wrapper)"
else
    echo "warning: x86_64-w64-mingw32-cmake not found. falling back to standard cmake with manual toolchain."
    cat > mingw-toolchain.cmake <<EOF
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF
    CMAKE_CMD="cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake"
fi

$CMAKE_CMD ..

cmake --build . -j$(nproc)

echo "copying required DLLs..."
DLL_DIR="/usr/x86_64-w64-mingw32/bin"
if [ -d "$DLL_DIR" ]; then
    cp "$DLL_DIR"/*.dll ./
    echo "  copied all DLLs from $DLL_DIR"
else
    echo "warning: DLL directory not found at $DLL_DIR"
fi

if [ -d "../assets" ]; then
    cp -r ../assets ./
    echo "assets copied."
else
    echo "warning: assets folder not found."
fi

echo ""
echo "done."

wine super_bert_bros.exe
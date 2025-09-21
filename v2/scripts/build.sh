#!/bin/bash
# Build libpolycall v2 - Pure interface layer

set -e

# Clean previous builds
rm -rf build/

# Create build directories
mkdir -p build/{unix,macos,windows}

# Detect platform and build
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    cd build/unix
    cmake ../.. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    cd build/macos
    cmake ../.. -DCMAKE_BUILD_TYPE=Release
    make -j$(sysctl -n hw.ncpu)
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    cd build/windows
    cmake ../.. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    mingw32-make -j
fi

echo "Build complete. Libraries in: build/$(uname -s)/lib/"

#!/bin/bash
# build.sh - Clean build script, no code generation

set -e

echo "Building PolyCall..."

# Clean previous build
make clean

# Build libraries
make all

# Run tests if requested
if [[ "$1" == "--test" ]]; then
    make test
fi

echo "Build complete. Libraries in lib/"
ls -la lib/

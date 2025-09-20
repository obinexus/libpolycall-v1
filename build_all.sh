#!/bin/bash
# OBINexus libpolycall - Build All Versions

set -e

echo "=== Building OBINexus libpolycall ==="

# Build v1 (if exists)
if [ -d "v1" ]; then
    echo ""
    echo "Building v1..."
    cd v1
    if [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
        make
        echo "✓ v1 built successfully"
    fi
    cd ..
fi

# Build v2
if [ -d "v2" ]; then
    echo ""
    echo "Building v2..."
    cd v2
    
    # Try different makefiles in order of preference
    if [ -f "Makefile.final" ]; then
        make -f Makefile.final clean
        make -f Makefile.final
    elif [ -f "Makefile.clean" ]; then
        make -f Makefile.clean clean
        make -f Makefile.clean
    elif [ -f "Makefile.direct" ]; then
        make -f Makefile.direct clean
        make -f Makefile.direct
    elif [ -f "Makefile" ]; then
        make clean
        make
    else
        echo "No Makefile found in v2"
        exit 1
    fi
    
    echo "✓ v2 built successfully"
    cd ..
fi

echo ""
echo "=== Build Complete ==="
echo "Libraries location:"
[ -f "v1/libpolycall.a" ] && echo "  v1: v1/libpolycall.a"
[ -f "v2/build/lib/libpolycall.a" ] && echo "  v2: v2/build/lib/libpolycall.a"
[ -f "v2/build/lib/libpolycall.so" ] && echo "  v2: v2/build/lib/libpolycall.so"

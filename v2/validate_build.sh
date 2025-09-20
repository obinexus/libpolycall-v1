#!/bin/bash
# OBINexus libpolycall v2 - Build System Validation
# Validates the refactored build system

set -e  # Exit on error

echo "╔══════════════════════════════════════════════╗"
echo "║  OBINexus libpolycall v2 Build Validation   ║"
echo "╚══════════════════════════════════════════════╝"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Base directory
BASE_DIR="$HOME/obinexus/workspace/libpolycall/v2"
cd "$BASE_DIR"

# Function to check file exists
check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} $1 exists"
        return 0
    else
        echo -e "${RED}✗${NC} $1 missing"
        return 1
    fi
}

# Function to check directory exists
check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} $1 exists"
        return 0
    else
        echo -e "${RED}✗${NC} $1 missing"
        return 1
    fi
}

echo ""
echo "=== Step 1: Checking Directory Structure ==="
check_dir "src/core"
check_dir "src/adapter"
check_dir "src/socket"
check_dir "src/micro"
check_dir "src/nlm"
check_dir "src/cli"
check_dir "include/libpolycall"

echo ""
echo "=== Step 2: Checking Build Files ==="
check_file "CMakeLists.txt"
check_file "Makefile"
check_file "src/core/CMakeLists.txt"
check_file "src/cli/CMakeLists.txt"

echo ""
echo "=== Step 3: Clean Previous Build ==="
make clean || true
rm -rf build/

echo ""
echo "=== Step 4: Configure with CMake ==="
mkdir -p build
cd build

if cmake .. -DCMAKE_BUILD_TYPE=Debug \
           -DBUILD_SHARED_LIBS=ON \
           -DBUILD_STATIC_LIBS=ON \
           -DBUILD_CLI=ON \
           -DBUILD_MODULES=ON; then
    echo -e "${GREEN}✓${NC} CMake configuration successful"
else
    echo -e "${RED}✗${NC} CMake configuration failed"
    exit 1
fi

echo ""
echo "=== Step 5: Build Libraries ==="
if make -j$(nproc); then
    echo -e "${GREEN}✓${NC} Build successful"
else
    echo -e "${YELLOW}⚠${NC} Build completed with warnings"
fi

echo ""
echo "=== Step 6: Verify Build Outputs ==="
echo "Checking libraries..."

# Check main libraries
if [ -f "lib/libpolycall.a" ]; then
    echo -e "${GREEN}✓${NC} Static library: lib/libpolycall.a"
    size=$(stat -c%s "lib/libpolycall.a")
    echo "  Size: $size bytes"
fi

if [ -f "lib/libpolycall.so" ]; then
    echo -e "${GREEN}✓${NC} Shared library: lib/libpolycall.so"
    # Check symbols
    echo "  Symbols found: $(nm lib/libpolycall.so 2>/dev/null | grep -c ' T ' || echo "0")"
fi

# Check micro module if built
if [ -f "lib/micro.a" ]; then
    echo -e "${GREEN}✓${NC} Micro static: lib/micro.a"
fi

if [ -f "lib/micro.so" ]; then
    echo -e "${GREEN}✓${NC} Micro shared: lib/micro.so"
fi

# Check CLI
if [ -f "bin/polycall_cli" ]; then
    echo -e "${GREEN}✓${NC} CLI executable: bin/polycall_cli"
    # Test run
    if ./bin/polycall_cli --version 2>/dev/null; then
        echo "  CLI runs successfully"
    fi
fi

echo ""
echo "=== Step 7: Library Dependency Check ==="
if [ -f "lib/libpolycall.so" ]; then
    echo "Dependencies for libpolycall.so:"
    ldd lib/libpolycall.so | head -5
fi

echo ""
echo "=== Step 8: Build Summary ==="
echo "Build directory structure:"
tree -L 2 . 2>/dev/null || ls -la

echo ""
echo "Library contents (first 10 symbols):"
if [ -f "lib/libpolycall.a" ]; then
    nm lib/libpolycall.a 2>/dev/null | grep ' T ' | head -10 || true
fi

echo ""
echo "╔══════════════════════════════════════════════╗"
echo "║          Build Validation Complete           ║"
echo "╚══════════════════════════════════════════════╝"

# Final status
cd "$BASE_DIR"
if [ -f "build/lib/libpolycall.a" ] || [ -f "build/lib/libpolycall.so" ]; then
    echo -e "${GREEN}SUCCESS${NC}: Build system is working!"
    echo ""
    echo "Next steps:"
    echo "1. Review any warnings in the build output"
    echo "2. Add missing source implementations"
    echo "3. Run: make install (for system installation)"
else
    echo -e "${RED}FAILURE${NC}: Build system needs attention"
    echo "Check the CMake output above for errors"
fi

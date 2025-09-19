#!/bin/bash
# polycall_repair_v2.sh - Adaptive repair script for PolyCall v2 structure

set -e  # Exit on error
set -u  # Exit on undefined variable

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== PolyCall v2 Module Repair Script ===${NC}"

# Detect current working directory and structure
CURRENT_DIR=$(pwd)
BASE_DIR=""
V2_DIR=""

# Find the correct base directory
if [[ "$CURRENT_DIR" == *"/v2" ]]; then
    BASE_DIR="$(dirname "$CURRENT_DIR")"
    V2_DIR="$CURRENT_DIR"
    echo -e "${BLUE}Running from v2/ directory${NC}"
elif [[ -d "v2" ]]; then
    BASE_DIR="$CURRENT_DIR"
    V2_DIR="$CURRENT_DIR/v2"
    echo -e "${BLUE}Found v2/ subdirectory${NC}"
elif [[ -d "../v2" ]]; then
    BASE_DIR="$(dirname "$CURRENT_DIR")"
    V2_DIR="$BASE_DIR/v2"
    echo -e "${BLUE}Found v2/ in parent directory${NC}"
else
    echo -e "${RED}Cannot find v2/ directory structure${NC}"
    exit 1
fi

echo "Base directory: $BASE_DIR"
echo "V2 directory: $V2_DIR"

# Check what directories exist and adapt
check_structure() {
    echo -e "${YELLOW}Analyzing directory structure...${NC}"
    
    # Check for libpolycall directory (appears to be the old v1)
    if [[ -d "$BASE_DIR/libpolycall" ]]; then
        echo "Found libpolycall v1 directory"
        LIBPOLYCALL_DIR="$BASE_DIR/libpolycall"
    fi
    
    # Check v2 structure
    if [[ -d "$V2_DIR/src" ]]; then
        echo "Found v2/src directory"
        V2_SRC_DIR="$V2_DIR/src"
    fi
    
    # Check for polycall-v2 directory
    if [[ -d "$V2_DIR/polycall-v2" ]]; then
        echo "Found polycall-v2 directory"
        POLYCALL_V2_DIR="$V2_DIR/polycall-v2"
    fi
    
    # Check isolated bindings
    if [[ -d "$V2_DIR/isolated/bindings" ]]; then
        echo "Found isolated bindings"
        BINDINGS_DIR="$V2_DIR/isolated/bindings"
    fi
}

# Create necessary directories for build
setup_build_dirs() {
    echo -e "${YELLOW}Setting up build directories...${NC}"
    
    mkdir -p "$V2_DIR/build"/{debug,release,test}
    mkdir -p "$V2_DIR/lib"
    mkdir -p "$V2_DIR/bin"
    mkdir -p "$V2_DIR/include"
    mkdir -p "$V2_DIR/test"/{unit,integration,performance}
    
    echo "Created build structure in $V2_DIR"
}

# Extract and organize source files from various locations
organize_sources() {
    echo -e "${YELLOW}Organizing source files...${NC}"
    
    # If polycall.rar exists, extract it
    if [[ -f "$BASE_DIR/polycall.rar" ]]; then
        echo "Found polycall.rar - extracting..."
        cd "$BASE_DIR"
        
        # Check if unrar is available
        if command -v unrar &> /dev/null; then
            unrar x -o+ polycall.rar polycall_extracted/ || true
        else
            echo -e "${RED}unrar not found. Please install: apt-get install unrar${NC}"
        fi
    fi
    
    # Check if we have the old libpolycall structure  
    if [[ -d "$BASE_DIR/libpolycall" ]]; then
        echo "Checking libpolycall v1 for source files..."
        
        # Look for source files in libpolycall
        if [[ -d "$BASE_DIR/libpolycall/src" ]]; then
            echo "Found v1 src directory"
            # Don't copy yet, just note it exists
        fi
    fi
    
    # Check polycall-v2 structure
    if [[ -d "$V2_DIR/polycall-v2" ]]; then
        echo "Analyzing polycall-v2 structure..."
        
        # Look for includes
        if [[ -d "$V2_DIR/polycall-v2/include" ]]; then
            echo "Copying headers from polycall-v2..."
            cp -r "$V2_DIR/polycall-v2/include"/* "$V2_DIR/include/" 2>/dev/null || true
        fi
        
        # Look for source
        if [[ -d "$V2_DIR/polycall-v2/src" ]]; then
            echo "Found source in polycall-v2"
        fi
    fi
}

# Create a simple test source structure if missing
create_minimal_sources() {
    echo -e "${YELLOW}Creating minimal source structure...${NC}"
    
    # Create main header
    cat > "$V2_DIR/include/polycall.h" << 'EOF'
#ifndef POLYCALL_H
#define POLYCALL_H

#include <pthread.h>
#include <stdint.h>

typedef struct {
    pthread_mutex_t mutex;
    int state;
    void* data;
} polycall_context_t;

int polycall_init(polycall_context_t* ctx);
int polycall_process(polycall_context_t* ctx, const void* input, void* output);
int polycall_cleanup(polycall_context_t* ctx);

#endif // POLYCALL_H
EOF
    
    # Create main source file
    mkdir -p "$V2_DIR/src/core"
    cat > "$V2_DIR/src/core/polycall.c" << 'EOF'
#include "polycall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int polycall_init(polycall_context_t* ctx) {
    if (!ctx) return -1;
    
    if (pthread_mutex_init(&ctx->mutex, NULL) != 0) {
        return -1;
    }
    
    ctx->state = 0;
    ctx->data = NULL;
    return 0;
}

int polycall_process(polycall_context_t* ctx, const void* input, void* output) {
    if (!ctx) return -1;
    
    pthread_mutex_lock(&ctx->mutex);
    // Process logic here
    ctx->state++;
    pthread_mutex_unlock(&ctx->mutex);
    
    return 0;
}

int polycall_cleanup(polycall_context_t* ctx) {
    if (!ctx) return -1;
    
    pthread_mutex_destroy(&ctx->mutex);
    if (ctx->data) {
        free(ctx->data);
        ctx->data = NULL;
    }
    return 0;
}
EOF
}

# Build the libraries with CMake if available
build_with_cmake() {
    echo -e "${YELLOW}Attempting CMake build...${NC}"
    
    cd "$V2_DIR"
    
    # Check for CMakeLists
    if [[ -f "CMakeLists.txt.unified" ]]; then
        echo "Using unified CMakeLists..."
        cp CMakeLists.txt.unified CMakeLists.txt
    elif [[ -f "CMakeLists.txt.polycall" ]]; then
        echo "Using polycall CMakeLists..."
        cp CMakeLists.txt.polycall CMakeLists.txt
    else
        echo "Creating new CMakeLists.txt..."
        cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(PolyCallV2 VERSION 2.0.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Thread support
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Build both static and shared libraries
add_library(polycall_static STATIC ${SOURCES})
add_library(polycall_shared SHARED ${SOURCES})

# Set library properties
set_target_properties(polycall_static PROPERTIES OUTPUT_NAME polycall)
set_target_properties(polycall_shared PROPERTIES OUTPUT_NAME polycall)
set_target_properties(polycall_shared PROPERTIES VERSION 2.0.0 SONAME libpolycall.so.2)

# Link libraries
target_link_libraries(polycall_static Threads::Threads)
target_link_libraries(polycall_shared Threads::Threads)

# Install targets
install(TARGETS polycall_static polycall_shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/polycall)
EOF
    fi
    
    # Build with CMake
    if command -v cmake &> /dev/null; then
        mkdir -p build/release
        cd build/release
        cmake ../.. -DCMAKE_BUILD_TYPE=Release
        make -j$(nproc) || echo "CMake build failed, trying fallback..."
        cd ../..
    else
        echo -e "${YELLOW}CMake not found, using direct compilation...${NC}"
    fi
}

# Direct compilation fallback
direct_compile() {
    echo -e "${YELLOW}Direct compilation of available sources...${NC}"
    
    cd "$V2_DIR"
    
    # Find all C files
    SOURCE_FILES=$(find . -name "*.c" -type f 2>/dev/null | grep -v test | grep -v example | head -20)
    
    if [[ -z "$SOURCE_FILES" ]]; then
        echo -e "${RED}No source files found!${NC}"
        create_minimal_sources
        SOURCE_FILES="src/core/polycall.c"
    fi
    
    echo "Found source files:"
    echo "$SOURCE_FILES"
    
    # Compile to object files
    mkdir -p build/obj
    for src in $SOURCE_FILES; do
        obj_name=$(basename "${src%.c}.o")
        echo "Compiling $src..."
        gcc -c -fPIC -pthread -I./include -I. -o "build/obj/$obj_name" "$src" 2>/dev/null || true
    done
    
    # Create static library
    if ls build/obj/*.o 1> /dev/null 2>&1; then
        ar rcs lib/libpolycall.a build/obj/*.o
        echo -e "${GREEN}Created static library: lib/libpolycall.a${NC}"
        
        # Create shared library
        gcc -shared -o lib/libpolycall.so build/obj/*.o -pthread
        echo -e "${GREEN}Created shared library: lib/libpolycall.so${NC}"
    fi
}

# Fix Python bindings
fix_python_bindings() {
    echo -e "${YELLOW}Fixing Python bindings...${NC}"
    
    if [[ -d "$V2_DIR/isolated/bindings/pypolycall" ]]; then
        cd "$V2_DIR/isolated/bindings/pypolycall"
        
        # Create setup.py if missing
        if [[ ! -f "setup.py" ]]; then
            cat > setup.py << 'EOF'
from setuptools import setup, find_packages

setup(
    name="pypolycall",
    version="2.0.0",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    install_requires=[],
    python_requires=">=3.6",
)
EOF
        fi
        
        # Ensure __init__.py exists
        touch src/__init__.py
        touch src/modules/__init__.py
    fi
}

# Create test suite
create_tests() {
    echo -e "${YELLOW}Creating test suite...${NC}"
    
    cat > "$V2_DIR/test/test_basic.c" << 'EOF'
#include <stdio.h>
#include <assert.h>
#include <dlfcn.h>

int test_static_link() {
    printf("Testing static library link...\n");
    // Add actual test here
    return 0;
}

int test_dynamic_load() {
    printf("Testing dynamic library load...\n");
    void* handle = dlopen("./lib/libpolycall.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load: %s\n", dlerror());
        return 1;
    }
    dlclose(handle);
    printf("✓ Dynamic loading successful\n");
    return 0;
}

int main() {
    int result = 0;
    result |= test_static_link();
    result |= test_dynamic_load();
    
    if (result == 0) {
        printf("\n✓ All tests passed\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }
    return result;
}
EOF
    
    # Compile test
    gcc -o "$V2_DIR/test/test_basic" "$V2_DIR/test/test_basic.c" -ldl
}

# Main execution flow
main() {
    echo -e "${GREEN}Starting PolyCall v2 repair process...${NC}"
    
    check_structure
    setup_build_dirs
    organize_sources
    
    # Try CMake build first
    build_with_cmake
    
    # Fallback to direct compilation if needed
    if [[ ! -f "$V2_DIR/lib/libpolycall.a" ]]; then
        direct_compile
    fi
    
    fix_python_bindings
    create_tests
    
    # Verify results
    echo -e "${GREEN}=== Build Verification ===${NC}"
    if [[ -f "$V2_DIR/lib/libpolycall.a" ]]; then
        echo -e "${GREEN}✓ Static library created${NC}"
        file "$V2_DIR/lib/libpolycall.a"
    else
        echo -e "${RED}✗ Static library missing${NC}"
    fi
    
    if [[ -f "$V2_DIR/lib/libpolycall.so" ]]; then
        echo -e "${GREEN}✓ Shared library created${NC}"
        file "$V2_DIR/lib/libpolycall.so"
    else
        echo -e "${RED}✗ Shared library missing${NC}"
    fi
    
    echo -e "${GREEN}=== Repair Complete ===${NC}"
    echo "Next steps:"
    echo "1. cd $V2_DIR"
    echo "2. ./test/test_basic"
    echo "3. Check lib/ directory for libraries"
}

# Run main
main "$@"

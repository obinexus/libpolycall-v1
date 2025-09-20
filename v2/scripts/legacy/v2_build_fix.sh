#!/bin/bash
# OBINexus libpolycall v2 - Build System Refactor Script
# This script fixes the entire v2 build structure

echo "=== OBINexus libpolycall v2 Build System Refactor ==="
cd ~/obinexus/workspace/libpolycall/v2

# 1. Fix src/cli/CMakeLists.txt
cat > src/cli/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - CLI component
# Fixed: removed non-existent pid_manager.c reference

# Collect actual CLI sources
set(CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/polycall_cli.c
)

# Only add executable if sources exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/polycall_cli.c")
    add_executable(polycall_cli ${CLI_SOURCES})
    
    # Link with main library
    if(TARGET polycall_shared)
        target_link_libraries(polycall_cli polycall_shared)
    elseif(TARGET polycall_static)
        target_link_libraries(polycall_cli polycall_static)
    endif()
    
    # Set output directory
    set_target_properties(polycall_cli PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    # Installation
    install(TARGETS polycall_cli DESTINATION bin)
else()
    message(WARNING "CLI source not found, skipping CLI build")
endif()
EOF

# 2. Create module-specific CMakeLists.txt for each component
# Micro module (creates micro.so and micro.a)
cat > src/micro/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - micro feature
# Builds both micro.so and micro.a

file(GLOB micro_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER micro_SOURCES EXCLUDE REGEX ".*CMake.*")

if(micro_SOURCES)
    # Static library: micro.a
    add_library(micro_static STATIC ${micro_SOURCES})
    set_target_properties(micro_static PROPERTIES
        OUTPUT_NAME "micro"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    
    # Shared library: micro.so
    add_library(micro_shared SHARED ${micro_SOURCES})
    set_target_properties(micro_shared PROPERTIES
        OUTPUT_NAME "micro"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    
    # Export sources to parent
    set(micro_SOURCES ${micro_SOURCES} PARENT_SCOPE)
    message(STATUS "Micro module: ${micro_SOURCES}")
endif()
EOF

# 3. Fix adapter module CMakeLists.txt
cat > src/adapter/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - adapter feature

file(GLOB adapter_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER adapter_SOURCES EXCLUDE REGEX ".*CMake.*")

if(adapter_SOURCES)
    set(adapter_SOURCES ${adapter_SOURCES} PARENT_SCOPE)
    message(STATUS "Adapter module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
EOF

# 4. Fix socket module CMakeLists.txt
cat > src/socket/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - socket feature

file(GLOB socket_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER socket_SOURCES EXCLUDE REGEX ".*CMake.*")

if(socket_SOURCES)
    set(socket_SOURCES ${socket_SOURCES} PARENT_SCOPE)
    message(STATUS "Socket module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
EOF

# 5. Fix hotwire module CMakeLists.txt
cat > src/hotwire/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - hotwire feature

file(GLOB hotwire_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER hotwire_SOURCES EXCLUDE REGEX ".*CMake.*")

if(hotwire_SOURCES)
    set(hotwire_SOURCES ${hotwire_SOURCES} PARENT_SCOPE)
    message(STATUS "Hotwire module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
EOF

# 6. Fix NLM module CMakeLists.txt
cat > src/nlm/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - NLM-Atlas feature

file(GLOB nlm_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER nlm_SOURCES EXCLUDE REGEX ".*CMake.*")
list(FILTER nlm_SOURCES EXCLUDE REGEX ".*altas.*")  # Remove typo version

if(nlm_SOURCES)
    set(nlm_SOURCES ${nlm_SOURCES} PARENT_SCOPE)
    message(STATUS "NLM module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
EOF

# 7. Create stream module CMakeLists.txt
cat > src/stream/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - stream feature

file(GLOB stream_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER stream_SOURCES EXCLUDE REGEX ".*CMake.*")

if(stream_SOURCES)
    set(stream_SOURCES ${stream_SOURCES} PARENT_SCOPE)
    message(STATUS "Stream module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
EOF

# 8. Create zero module CMakeLists.txt
cat > src/zero/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - zero feature

file(GLOB zero_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER zero_SOURCES EXCLUDE REGEX ".*CMake.*")

if(zero_SOURCES)
    set(zero_SOURCES ${zero_SOURCES} PARENT_SCOPE)
    message(STATUS "Zero module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
EOF

# 9. Create main root CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - Root Build Configuration
cmake_minimum_required(VERSION 3.10)
project(libpolycall VERSION 2.0.0 LANGUAGES C)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build options
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_CLI "Build CLI executable" ON)
option(BUILD_MODULES "Build individual module libraries" ON)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Thread support
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/libpolycall
    ${CMAKE_SOURCE_DIR}/include/polycall
)

# Process feature modules
set(ALL_SOURCES "")

# Core module (required)
if(EXISTS "${CMAKE_SOURCE_DIR}/src/core")
    add_subdirectory(src/core)
    list(APPEND ALL_SOURCES ${core_SOURCES})
endif()

# Optional modules
foreach(module adapter socket hotwire micro nlm stream zero)
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/${module}")
        add_subdirectory(src/${module})
        if(${module}_SOURCES)
            list(APPEND ALL_SOURCES ${${module}_SOURCES})
        endif()
    endif()
endforeach()

# Remove duplicates and filter
list(REMOVE_DUPLICATES ALL_SOURCES)
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main\\.c")

# Build main polycall libraries
if(BUILD_STATIC_LIBS AND ALL_SOURCES)
    add_library(polycall_static STATIC ${ALL_SOURCES})
    target_link_libraries(polycall_static Threads::Threads)
    set_target_properties(polycall_static PROPERTIES OUTPUT_NAME "polycall")
    
    # Install static library
    install(TARGETS polycall_static DESTINATION lib)
endif()

if(BUILD_SHARED_LIBS AND ALL_SOURCES)
    add_library(polycall_shared SHARED ${ALL_SOURCES})
    target_link_libraries(polycall_shared Threads::Threads)
    set_target_properties(polycall_shared PROPERTIES 
        OUTPUT_NAME "polycall"
        VERSION ${PROJECT_VERSION}
        SOVERSION 2
    )
    
    # Install shared library
    install(TARGETS polycall_shared DESTINATION lib)
endif()

# Build CLI if requested
if(BUILD_CLI AND EXISTS "${CMAKE_SOURCE_DIR}/src/cli")
    add_subdirectory(src/cli)
endif()

# Install headers
install(DIRECTORY include/ DESTINATION include)

# Status report
message(STATUS "===========================================")
message(STATUS "OBINexus libpolycall v2 Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  Static libs: ${BUILD_STATIC_LIBS}")
message(STATUS "  CLI: ${BUILD_CLI}")
message(STATUS "  Source files found: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Module count: 8")
message(STATUS "===========================================")
EOF

# 10. Create improved Makefile
cat > Makefile << 'EOF'
# OBINexus libpolycall v2 - Main Makefile
# Orchestrates CMake build process

BUILD_DIR = build
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Release

.PHONY: all clean configure build install test debug release

all: configure build

configure:
	@echo "=== Configuring OBINexus libpolycall v2 ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) ..

build: configure
	@echo "=== Building OBINexus libpolycall v2 ==="
	@cd $(BUILD_DIR) && $(MAKE) -j$(shell nproc)
	@echo "=== Build Complete ==="
	@echo "Libraries: $(BUILD_DIR)/lib/"
	@echo "Binaries: $(BUILD_DIR)/bin/"

debug:
	@$(MAKE) CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug" all

release:
	@$(MAKE) CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release" all

install: build
	@echo "=== Installing OBINexus libpolycall v2 ==="
	@cd $(BUILD_DIR) && $(MAKE) install

clean:
	@echo "=== Cleaning build artifacts ==="
	@rm -rf $(BUILD_DIR)
	@rm -f lib/*.a lib/*.so* bin/*
	@echo "Clean complete"

# Create standard build directories
dirs:
	@mkdir -p build/obj build/lib build/bin
	@mkdir -p lib bin

# Show build status
status:
	@echo "=== Build Status ==="
	@echo "Static libraries:"
	@ls -la $(BUILD_DIR)/lib/*.a 2>/dev/null || echo "  None built"
	@echo "Shared libraries:"
	@ls -la $(BUILD_DIR)/lib/*.so* 2>/dev/null || echo "  None built"
	@echo "Executables:"
	@ls -la $(BUILD_DIR)/bin/* 2>/dev/null || echo "  None built"

# Verify build
verify: build
	@echo "=== Verifying Build ==="
	@file $(BUILD_DIR)/lib/libpolycall.* 2>/dev/null || echo "Libraries not found"
	@ldd $(BUILD_DIR)/lib/libpolycall.so 2>/dev/null || echo "Shared library check skipped"
	@nm $(BUILD_DIR)/lib/libpolycall.a 2>/dev/null | head -5 || echo "Static library check skipped"

# Quick test build
test-build:
	@echo "=== Test Build ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DBUILD_CLI=OFF ..
	@cd $(BUILD_DIR) && $(MAKE) -j2
	@ls -la $(BUILD_DIR)/lib/
EOF

# 11. Fix core module CMakeLists.txt
cat > src/core/CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - core module

file(GLOB core_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Filter out test files and CMake artifacts
list(FILTER core_SOURCES EXCLUDE REGEX ".*test.*")
list(FILTER core_SOURCES EXCLUDE REGEX ".*CMake.*")
list(FILTER core_SOURCES EXCLUDE REGEX ".*main\\.c")

if(core_SOURCES)
    set(core_SOURCES ${core_SOURCES} PARENT_SCOPE)
    message(STATUS "Core module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
    
    # Build standalone core library if requested
    if(BUILD_MODULES)
        add_library(core_module STATIC ${core_SOURCES})
        set_target_properties(core_module PROPERTIES
            OUTPUT_NAME "polycall_core"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/modules"
        )
    endif()
endif()
EOF

# 12. Create missing source files for CLI if needed
if [ ! -f src/cli/polycall_cli.c ]; then
    echo "Creating placeholder CLI source..."
    cat > src/cli/polycall_cli.c << 'EOF'
/*
 * OBINexus libpolycall v2
 * Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("OBINexus libpolycall v2.0.0\n");
    printf("Usage: %s [options]\n", argv[0]);
    
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printf("Version: 2.0.0\n");
        printf("Build: CMake/Make\n");
        return 0;
    }
    
    printf("CLI implementation pending...\n");
    return 0;
}
EOF
fi

echo "=== Build system refactor complete ==="
echo ""
echo "To build:"
echo "  make clean"
echo "  make"
echo ""
echo "Build outputs will be in:"
echo "  build/lib/ - Libraries (libpolycall.a, libpolycall.so, micro.a, micro.so)"
echo "  build/bin/ - Executables (polycall_cli)"
echo ""
echo "For module-specific builds:"
echo "  cmake -DBUILD_MODULES=ON .."

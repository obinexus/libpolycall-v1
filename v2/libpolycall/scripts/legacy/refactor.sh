#!/bin/bash
# OBINexus libpolycall v2 - Hierarchical CMake Build System Generator
# Creates feature-based modular CMakeLists.txt structure

set -e

echo "=== OBINexus CMake Hierarchy Generator ==="
echo "Building feature-based modular compilation system..."

# Create root CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - Root CMake Configuration
cmake_minimum_required(VERSION 3.15)
project(libpolycall VERSION 2.0.0 LANGUAGES C)

# Global configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(ENABLE_FEATURES "Enable feature-based build" ON)

# Platform detection
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(LIB_SUFFIX "dylib")
elseif(UNIX)
    set(LIB_SUFFIX "so")
elseif(WIN32)
    set(LIB_SUFFIX "dll")
endif()

# Thread support
find_package(Threads REQUIRED)

# Feature detection from XML
function(parse_features)
    file(READ "${CMAKE_SOURCE_DIR}/config/features.xml" FEATURES_XML)
    string(REGEX MATCHALL "<feature name=\"([^\"]+)\" enabled=\"true\">" ENABLED_FEATURES "${FEATURES_XML}")
    set(POLYCALL_FEATURES "" PARENT_SCOPE)
    foreach(FEATURE_MATCH ${ENABLED_FEATURES})
        string(REGEX REPLACE "<feature name=\"([^\"]+)\" enabled=\"true\">" "\\1" FEATURE_NAME "${FEATURE_MATCH}")
        list(APPEND POLYCALL_FEATURES ${FEATURE_NAME})
    endforeach()
    set(POLYCALL_FEATURES ${POLYCALL_FEATURES} PARENT_SCOPE)
endfunction()

# Parse enabled features
if(ENABLE_FEATURES AND EXISTS "${CMAKE_SOURCE_DIR}/config/features.xml")
    parse_features()
    message(STATUS "Enabled features: ${POLYCALL_FEATURES}")
else()
    # Default features if XML parsing fails
    set(POLYCALL_FEATURES core adapter hotwire socket micro nlm stream zero)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/libpolycall
)

# Process feature modules
set(ALL_FEATURE_SOURCES)
set(ALL_FEATURE_HEADERS)

foreach(FEATURE ${POLYCALL_FEATURES})
    set(FEATURE_DIR "${CMAKE_SOURCE_DIR}/src/${FEATURE}")
    if(EXISTS "${FEATURE_DIR}/CMakeLists.txt")
        add_subdirectory("${FEATURE_DIR}")
        list(APPEND ALL_FEATURE_SOURCES ${${FEATURE}_SOURCES})
        list(APPEND ALL_FEATURE_HEADERS ${${FEATURE}_HEADERS})
    elseif(EXISTS "${FEATURE_DIR}")
        # Direct source collection if no CMakeLists.txt
        file(GLOB FEATURE_SOURCES "${FEATURE_DIR}/*.c")
        file(GLOB FEATURE_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/${FEATURE}/*.h")
        list(APPEND ALL_FEATURE_SOURCES ${FEATURE_SOURCES})
        list(APPEND ALL_FEATURE_HEADERS ${FEATURE_HEADERS})
    endif()
endforeach()

# Core library sources (always included)
file(GLOB CORE_SOURCES "src/core/*.c")
list(APPEND ALL_FEATURE_SOURCES ${CORE_SOURCES})

# Build targets
if(BUILD_STATIC_LIBS)
    add_library(polycall_static STATIC ${ALL_FEATURE_SOURCES})
    target_link_libraries(polycall_static PRIVATE Threads::Threads)
    set_target_properties(polycall_static PROPERTIES
        OUTPUT_NAME "polycall"
        PREFIX "lib"
        CLEAN_DIRECT_OUTPUT 1
    )
endif()

if(BUILD_SHARED_LIBS)
    add_library(polycall_shared SHARED ${ALL_FEATURE_SOURCES})
    target_link_libraries(polycall_shared PRIVATE Threads::Threads)
    set_target_properties(polycall_shared PROPERTIES
        OUTPUT_NAME "polycall"
        VERSION ${PROJECT_VERSION}
        SOVERSION 2
        PREFIX "lib"
        CLEAN_DIRECT_OUTPUT 1
    )
endif()

# Installation
install(DIRECTORY include/libpolycall DESTINATION include)
if(BUILD_STATIC_LIBS)
    install(TARGETS polycall_static
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
endif()
if(BUILD_SHARED_LIBS)
    install(TARGETS polycall_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

# Export build configuration for Makefile integration
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/polycall-config.cmake.in"
    "${CMAKE_BINARY_DIR}/polycall-config.cmake"
    @ONLY
)
EOF

# Create feature template CMakeLists.txt
mkdir -p cmake
cat > cmake/feature-template.cmake << 'EOF'
# Feature: @FEATURE_NAME@
# Auto-generated CMakeLists.txt for feature module

# Collect sources for this feature
file(GLOB @FEATURE_NAME@_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB @FEATURE_NAME@_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/@FEATURE_NAME@/*.h")

# Export to parent scope
set(@FEATURE_NAME@_SOURCES ${@FEATURE_NAME@_SOURCES} PARENT_SCOPE)
set(@FEATURE_NAME@_HEADERS ${@FEATURE_NAME@_HEADERS} PARENT_SCOPE)

# Feature-specific compile definitions
if(@FEATURE_NAME@_SOURCES)
    foreach(SOURCE ${@FEATURE_NAME@_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_@FEATURE_NAME_UPPER@=1"
        )
    endforeach()
endif()
EOF

# Generate feature-specific CMakeLists.txt
for feature in core adapter hotwire socket micro nlm stream zero; do
    feature_dir="src/${feature}"
    mkdir -p "${feature_dir}"
    
    if [ ! -f "${feature_dir}/CMakeLists.txt" ]; then
        cat > "${feature_dir}/CMakeLists.txt" << EOF
# Feature: ${feature}
# OBINexus libpolycall v2 - ${feature} module

# Collect sources for ${feature}
file(GLOB ${feature}_SOURCES "\${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB ${feature}_HEADERS "\${CMAKE_SOURCE_DIR}/include/libpolycall/${feature}/*.h")

# Export to parent scope
set(${feature}_SOURCES \${${feature}_SOURCES} PARENT_SCOPE)
set(${feature}_HEADERS \${${feature}_HEADERS} PARENT_SCOPE)

# Feature-specific definitions
if(${feature}_SOURCES)
    foreach(SOURCE \${${feature}_SOURCES})
        set_source_files_properties(\${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_${feature^^}=1"
        )
    endforeach()
endif()

message(STATUS "Feature ${feature}: \${${feature}_SOURCES}")
EOF
    fi
done

# Create Makefile wrapper for CMake
cat > Makefile << 'EOF'
# OBINexus libpolycall v2 - Master Makefile
# Wraps CMake build for nlink/polybuild orchestration

BUILD_DIR ?= build
CMAKE ?= cmake
FEATURES ?= all

.PHONY: all clean configure build install test

all: configure build

configure:
	@echo "=== Configuring OBINexus libpolycall v2 ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=ON \
		-DBUILD_STATIC_LIBS=ON \
		-DENABLE_FEATURES=ON

build: configure
	@echo "=== Building libpolycall ==="
	@$(CMAKE) --build $(BUILD_DIR) --parallel

install: build
	@echo "=== Installing libpolycall ==="
	@$(CMAKE) --install $(BUILD_DIR)

clean:
	@echo "=== Cleaning build artifacts ==="
	@rm -rf $(BUILD_DIR)

# Direct make targets (bypass CMake)
native-build:
	@echo "=== Native build (no CMake) ==="
	$(MAKE) -f Makefile.native

# Feature-specific builds
feature-%:
	@echo "=== Building feature: $* ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DPOLYCALL_FEATURE_ONLY=$*
	@$(CMAKE) --build $(BUILD_DIR)

# Library output verification
verify:
	@echo "=== Verifying library outputs ==="
	@if [ -f "$(BUILD_DIR)/libpolycall.so" ] || [ -f "$(BUILD_DIR)/libpolycall.dylib" ]; then \
		echo "✓ Shared library built"; \
	else \
		echo "✗ Shared library missing"; \
	fi
	@if [ -f "$(BUILD_DIR)/libpolycall.a" ]; then \
		echo "✓ Static library built"; \
	else \
		echo "✗ Static library missing"; \
	fi

# Integration with nlink/polybuild
nlink:
	@echo "=== nlink orchestration ==="
	@./scripts/nlink-integrate.sh

polybuild:
	@echo "=== polybuild orchestration ==="
	@./scripts/polybuild-run.sh
EOF

# Create configuration export template
cat > cmake/polycall-config.cmake.in << 'EOF'
# OBINexus libpolycall v2 - Build Configuration Export
# Generated by CMake

set(POLYCALL_VERSION "@PROJECT_VERSION@")
set(POLYCALL_FEATURES "@POLYCALL_FEATURES@")
set(POLYCALL_INCLUDE_DIR "@CMAKE_INSTALL_PREFIX@/include")
set(POLYCALL_LIB_DIR "@CMAKE_INSTALL_PREFIX@/lib")

# Library names
set(POLYCALL_STATIC_LIB "libpolycall.a")
set(POLYCALL_SHARED_LIB "libpolycall.@LIB_SUFFIX@")

# Feature flags
foreach(FEATURE @POLYCALL_FEATURES@)
    set(POLYCALL_HAS_${FEATURE} TRUE)
endforeach()
EOF

echo "=== OBINexus CMake Hierarchy Complete ==="
echo ""
echo "Build system structure created:"
echo "  • Root CMakeLists.txt with feature parsing"
echo "  • Feature-specific CMakeLists.txt in src/[feature]/"
echo "  • Makefile wrapper for make commands"
echo "  • CMake configuration export"
echo ""
echo "Usage:"
echo "  make              # Full build with CMake"
echo "  make feature-core # Build specific feature"
echo "  make native-build # Direct compilation (no CMake)"
echo "  make verify       # Check library outputs"
echo "  make nlink        # nlink orchestration"
echo "  make polybuild    # polybuild integration"

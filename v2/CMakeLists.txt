# OBINexus libpolycall v2 - Pure Presentation Layer
cmake_minimum_required(VERSION 3.15)
project(libpolycall VERSION 2.0.0 LANGUAGES C)

# Enforce no business logic - pure interface layer
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Detect architecture/platform
if(APPLE)
    set(TARGET_ARCH "macos")
elseif(WIN32)
    set(TARGET_ARCH "windows")
else()
    set(TARGET_ARCH "unix")  # Linux, WSL, BSD, etc.
endif()

# Build output structure
set(BUILD_BASE_DIR ${CMAKE_BINARY_DIR}/${TARGET_ARCH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_BASE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_BASE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_BASE_DIR}/bin)

# Object files directory
set(OBJECT_OUTPUT_DIR ${BUILD_BASE_DIR}/obj)

# Thread support
find_package(Threads REQUIRED)

# Global includes
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/libpolycall
)

# Module registration
set(POLYCALL_MODULES
    core
    socket
    micro
    nlm
    adapter
    hotwire
    stream
    zero
)

# Build each module
foreach(MODULE ${POLYCALL_MODULES})
    if(EXISTS ${CMAKE_SOURCE_DIR}/src/${MODULE}/CMakeLists.txt)
        add_subdirectory(src/${MODULE})
    endif()
endforeach()

# CLI tool
if(EXISTS ${CMAKE_SOURCE_DIR}/src/cli/CMakeLists.txt)
    add_subdirectory(src/cli)
endif()

# Installation rules
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY ${BUILD_BASE_DIR}/lib/ DESTINATION lib)
install(DIRECTORY ${BUILD_BASE_DIR}/bin/ DESTINATION bin)

# Generate FFI loader config
configure_file(
    ${CMAKE_SOURCE_DIR}/config/libpolycall.ffi.json.in
    ${BUILD_BASE_DIR}/libpolycall.ffi.json
    @ONLY
)

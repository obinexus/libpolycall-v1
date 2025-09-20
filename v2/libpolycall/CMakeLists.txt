# OBINexus libpolycall v2 - Root CMake (Fixed)
cmake_minimum_required(VERSION 3.10)
project(libpolycall VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_CLI "Build CLI executable" ON)

# Thread support
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/libpolycall
)

# Collect library sources (excluding CLI and CMake artifacts)
file(GLOB_RECURSE ALL_SOURCES 
    "src/core/*.c"
    "src/adapter/*.c"
    "src/socket/*.c"
    "src/hotwire/*.c"
    "src/micro/*.c"
    "src/nlm/*.c"
    "src/stream/*.c"
    "src/zero/*.c"
)

# Filter out unwanted files
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*CMake.*")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main\\.c")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*nlm_altas.*")  # typo version

# Remove duplicates
list(REMOVE_DUPLICATES ALL_SOURCES)

# Build libraries
if(BUILD_STATIC_LIBS)
    add_library(polycall_static STATIC ${ALL_SOURCES})
    target_link_libraries(polycall_static Threads::Threads)
    set_target_properties(polycall_static PROPERTIES
        OUTPUT_NAME "polycall"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

if(BUILD_SHARED_LIBS)
    add_library(polycall_shared SHARED ${ALL_SOURCES})
    target_link_libraries(polycall_shared Threads::Threads)
    set_target_properties(polycall_shared PROPERTIES
        OUTPUT_NAME "polycall"
        VERSION ${PROJECT_VERSION}
        SOVERSION 2
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

# Build CLI separately
if(BUILD_CLI AND EXISTS "${CMAKE_SOURCE_DIR}/src/cli")
    add_subdirectory(src/cli)
endif()

# Installation
install(DIRECTORY include/ DESTINATION include)
if(TARGET polycall_static)
    install(TARGETS polycall_static DESTINATION lib)
endif()
if(TARGET polycall_shared)
    install(TARGETS polycall_shared DESTINATION lib)
endif()

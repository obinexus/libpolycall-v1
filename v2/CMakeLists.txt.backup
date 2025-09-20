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

# Output directories - proper structure
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Object files directory
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)

# Thread support
find_package(Threads REQUIRED)

# Include directories - simplified
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Collect sources by module
set(MODULE_LIBS "")

# Core module
file(GLOB CORE_SOURCES "src/core/*.c")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*CMake.*")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*main\\.c")

# Adapter module  
file(GLOB ADAPTER_SOURCES "src/adapter/*.c")
list(FILTER ADAPTER_SOURCES EXCLUDE REGEX ".*CMake.*")

# Socket module
file(GLOB SOCKET_SOURCES "src/socket/*.c")
list(FILTER SOCKET_SOURCES EXCLUDE REGEX ".*CMake.*")

# Micro module
file(GLOB MICRO_SOURCES "src/micro/*.c")
list(FILTER MICRO_SOURCES EXCLUDE REGEX ".*CMake.*")

# NLM module
file(GLOB NLM_SOURCES "src/nlm/*.c")
list(FILTER NLM_SOURCES EXCLUDE REGEX ".*CMake.*")

# Hotwire module
file(GLOB HOTWIRE_SOURCES "src/hotwire/*.c")
list(FILTER HOTWIRE_SOURCES EXCLUDE REGEX ".*CMake.*")

# Stream module
file(GLOB STREAM_SOURCES "src/stream/*.c")
list(FILTER STREAM_SOURCES EXCLUDE REGEX ".*CMake.*")

# Zero module
file(GLOB ZERO_SOURCES "src/zero/*.c")
list(FILTER ZERO_SOURCES EXCLUDE REGEX ".*CMake.*")

# All sources combined
set(ALL_SOURCES 
    ${CORE_SOURCES} 
    ${ADAPTER_SOURCES} 
    ${SOCKET_SOURCES} 
    ${MICRO_SOURCES}
    ${NLM_SOURCES}
    ${HOTWIRE_SOURCES}
    ${STREAM_SOURCES}
    ${ZERO_SOURCES}
)

# Remove any duplicates
list(REMOVE_DUPLICATES ALL_SOURCES)

# Build module-specific libraries if requested
if(BUILD_MODULES)
    # Core module library
    if(CORE_SOURCES)
        add_library(core_static STATIC ${CORE_SOURCES})
        add_library(core_shared SHARED ${CORE_SOURCES})
        set_target_properties(core_static PROPERTIES OUTPUT_NAME "polycall_core")
        set_target_properties(core_shared PROPERTIES OUTPUT_NAME "polycall_core")
        list(APPEND MODULE_LIBS core_static core_shared)
    endif()
    
    # Micro module library
    if(MICRO_SOURCES)
        add_library(micro_static STATIC ${MICRO_SOURCES})
        add_library(micro_shared SHARED ${MICRO_SOURCES})
        set_target_properties(micro_static PROPERTIES OUTPUT_NAME "micro")
        set_target_properties(micro_shared PROPERTIES OUTPUT_NAME "micro")
        list(APPEND MODULE_LIBS micro_static micro_shared)
    endif()
    
    # Adapter module library
    if(ADAPTER_SOURCES)
        add_library(adapter_static STATIC ${ADAPTER_SOURCES})
        add_library(adapter_shared SHARED ${ADAPTER_SOURCES})
        set_target_properties(adapter_static PROPERTIES OUTPUT_NAME "adapter")
        set_target_properties(adapter_shared PROPERTIES OUTPUT_NAME "adapter")
        list(APPEND MODULE_LIBS adapter_static adapter_shared)
    endif()
    
    # Socket module library
    if(SOCKET_SOURCES)
        add_library(socket_static STATIC ${SOCKET_SOURCES})
        add_library(socket_shared SHARED ${SOCKET_SOURCES})
        set_target_properties(socket_static PROPERTIES OUTPUT_NAME "socket")
        set_target_properties(socket_shared PROPERTIES OUTPUT_NAME "socket")
        list(APPEND MODULE_LIBS socket_static socket_shared)
    endif()
endif()

# Main polycall libraries
if(BUILD_STATIC_LIBS AND ALL_SOURCES)
    add_library(polycall_static STATIC ${ALL_SOURCES})
    target_link_libraries(polycall_static Threads::Threads)
    set_target_properties(polycall_static PROPERTIES OUTPUT_NAME "polycall")
endif()

if(BUILD_SHARED_LIBS AND ALL_SOURCES)
    add_library(polycall_shared SHARED ${ALL_SOURCES})
    target_link_libraries(polycall_shared Threads::Threads)
    set_target_properties(polycall_shared PROPERTIES 
        OUTPUT_NAME "polycall"
        VERSION ${PROJECT_VERSION}
        SOVERSION 2
    )
endif()

# CLI executable
if(BUILD_CLI AND EXISTS "${CMAKE_SOURCE_DIR}/src/cli/polycall_cli.c")
    add_executable(polycall_cli src/cli/polycall_cli.c)
    if(TARGET polycall_shared)
        target_link_libraries(polycall_cli polycall_shared)
    elseif(TARGET polycall_static)
        target_link_libraries(polycall_cli polycall_static)
    endif()
endif()

# Installation
install(DIRECTORY include/ DESTINATION include)
if(TARGET polycall_static)
    install(TARGETS polycall_static DESTINATION lib)
endif()
if(TARGET polycall_shared)
    install(TARGETS polycall_shared DESTINATION lib)
endif()
if(TARGET polycall_cli)
    install(TARGETS polycall_cli DESTINATION bin)
endif()

# Status
message(STATUS "=====================================")
message(STATUS "OBINexus libpolycall v2")
message(STATUS "  Modules found:")
if(CORE_SOURCES)
    message(STATUS "    - core: ${words ${CORE_SOURCES}} files")
endif()
if(MICRO_SOURCES)
    message(STATUS "    - micro: ${words ${MICRO_SOURCES}} files")
endif()
if(ADAPTER_SOURCES)
    message(STATUS "    - adapter: ${words ${ADAPTER_SOURCES}} files")
endif()
if(SOCKET_SOURCES)
    message(STATUS "    - socket: ${words ${SOCKET_SOURCES}} files")
endif()
message(STATUS "=====================================")

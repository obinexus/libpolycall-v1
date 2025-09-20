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

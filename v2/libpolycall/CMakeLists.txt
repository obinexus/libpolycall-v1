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

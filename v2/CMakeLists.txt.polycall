cmake_minimum_required(VERSION 3.15)
project(polycall-v2 VERSION 2.0.0 LANGUAGES C CXX)

# Configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(include lib/shared)

# Find source files
file(GLOB_RECURSE CORE_SOURCES "src/core/*.c")
file(GLOB_RECURSE CLI_SOURCES "src/cli/*.c")
file(GLOB_RECURSE FFI_SOURCES "src/ffi/*.c")

#
rget(qa_validation ALL
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/qa_suite_run.sh
    COMMENT "Running Polycall QA validation suite"
)

add_dependencies(${PROJECT_NAME} qa_validation)

# Ensure build fails if QA fails
add_custom_command(TARGET qa_validation
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Verifying build grade..."
    COMMAND test `cat ${CMAKE_BINARY_DIR}/build/metadata/build-grade.txt` -lt 7
    COMMENT "Validating fault grade constraints"
)

# Core library
add_library(polycall-core STATIC ${CORE_SOURCES})
target_compile_definitions(polycall-core PRIVATE POLYCALL_VERSION="${PROJECT_VERSION}")

# CLI executable
if(CLI_SOURCES)
    add_executable(polycall ${CLI_SOURCES})
    target_link_libraries(polycall polycall-core)
endif()

# FFI library
if(FFI_SOURCES)
    add_library(polycall-ffi SHARED ${FFI_SOURCES})
    target_link_libraries(polycall-ffi polycall-core)
endif()

# Platform-specific configuration
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_compile_definitions(polycall-core PRIVATE POLYCALL_FREEBSD)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(polycall-core PRIVATE POLYCALL_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(polycall-core PRIVATE POLYCALL_WINDOWS)
endif()

add_custom_ta
# Testing
if(BUILD_TESTING)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        add_subdirectory(tests)
    endif()
endif()

# Installation
install(TARGETS polycall-core DESTINATION lib)
if(TARGET polycall)
    install(TARGETS polycall DESTINATION bin)
endif()
if(TARGET polycall-ffi)
    install(TARGETS polycall-ffi DESTINATION lib)
endif()

# Package configuration
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
include(CPack)

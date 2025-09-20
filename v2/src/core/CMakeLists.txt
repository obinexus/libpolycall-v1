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

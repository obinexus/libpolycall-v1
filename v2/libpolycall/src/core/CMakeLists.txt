# OBINexus libpolycall v2 - core feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE core_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER core_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(core_SOURCES ${core_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_core OBJECT ${core_SOURCES})
    target_compile_definitions(polycall_core PRIVATE
        POLYCALL_FEATURE_CORE=1
    )
endif()

message(STATUS "Feature core: ${core_SOURCES}")

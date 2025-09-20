# OBINexus libpolycall v2 - micro feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE micro_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER micro_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(micro_SOURCES ${micro_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_micro OBJECT ${micro_SOURCES})
    target_compile_definitions(polycall_micro PRIVATE
        POLYCALL_FEATURE_MICRO=1
    )
endif()

message(STATUS "Feature micro: ${micro_SOURCES}")

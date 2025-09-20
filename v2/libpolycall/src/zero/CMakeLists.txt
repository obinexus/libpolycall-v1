# OBINexus libpolycall v2 - zero feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE zero_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER zero_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(zero_SOURCES ${zero_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_zero OBJECT ${zero_SOURCES})
    target_compile_definitions(polycall_zero PRIVATE
        POLYCALL_FEATURE_ZERO=1
    )
endif()

message(STATUS "Feature zero: ${zero_SOURCES}")

# OBINexus libpolycall v2 - nlm feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE nlm_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER nlm_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(nlm_SOURCES ${nlm_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_nlm OBJECT ${nlm_SOURCES})
    target_compile_definitions(polycall_nlm PRIVATE
        POLYCALL_FEATURE_NLM=1
    )
endif()

message(STATUS "Feature nlm: ${nlm_SOURCES}")

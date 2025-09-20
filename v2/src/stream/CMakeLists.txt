# OBINexus libpolycall v2 - stream feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE stream_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER stream_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(stream_SOURCES ${stream_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_stream OBJECT ${stream_SOURCES})
    target_compile_definitions(polycall_stream PRIVATE
        POLYCALL_FEATURE_STREAM=1
    )
endif()

message(STATUS "Feature stream: ${stream_SOURCES}")

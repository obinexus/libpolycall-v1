# OBINexus libpolycall v2 - adapter feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE adapter_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER adapter_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(adapter_SOURCES ${adapter_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_adapter OBJECT ${adapter_SOURCES})
    target_compile_definitions(polycall_adapter PRIVATE
        POLYCALL_FEATURE_ADAPTER=1
    )
endif()

message(STATUS "Feature adapter: ${adapter_SOURCES}")

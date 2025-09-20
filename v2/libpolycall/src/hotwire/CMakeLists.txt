# OBINexus libpolycall v2 - hotwire feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE hotwire_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER hotwire_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(hotwire_SOURCES ${hotwire_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_hotwire OBJECT ${hotwire_SOURCES})
    target_compile_definitions(polycall_hotwire PRIVATE
        POLYCALL_FEATURE_HOTWIRE=1
    )
endif()

message(STATUS "Feature hotwire: ${hotwire_SOURCES}")

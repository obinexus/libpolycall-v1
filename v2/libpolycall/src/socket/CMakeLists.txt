# OBINexus libpolycall v2 - socket feature
# Component-based build configuration

# Collect all sources in this feature
file(GLOB_RECURSE socket_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Remove CMake artifacts
list(FILTER socket_SOURCES EXCLUDE REGEX ".*CMake.*")

# Export to parent
set(socket_SOURCES ${socket_SOURCES} PARENT_SCOPE)

# Feature library (optional - for testing individual features)
if(BUILD_FEATURE_LIBS)
    add_library(polycall_socket OBJECT ${socket_SOURCES})
    target_compile_definitions(polycall_socket PRIVATE
        POLYCALL_FEATURE_SOCKET=1
    )
endif()

message(STATUS "Feature socket: ${socket_SOURCES}")

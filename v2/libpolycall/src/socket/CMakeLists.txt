# Feature: socket
# OBINexus libpolycall v2 - socket module

# Collect sources for socket
file(GLOB socket_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB socket_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/socket/*.h")

# Export to parent scope
set(socket_SOURCES ${socket_SOURCES} PARENT_SCOPE)
set(socket_HEADERS ${socket_HEADERS} PARENT_SCOPE)

# Feature-specific definitions
if(socket_SOURCES)
    foreach(SOURCE ${socket_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_SOCKET=1"
        )
    endforeach()
endif()

message(STATUS "Feature socket: ${socket_SOURCES}")

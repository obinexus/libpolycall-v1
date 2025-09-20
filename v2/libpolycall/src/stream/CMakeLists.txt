# Feature: stream
# OBINexus libpolycall v2 - stream module

# Collect sources for stream
file(GLOB stream_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB stream_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/stream/*.h")

# Export to parent scope
set(stream_SOURCES ${stream_SOURCES} PARENT_SCOPE)
set(stream_HEADERS ${stream_HEADERS} PARENT_SCOPE)

# Feature-specific definitions
if(stream_SOURCES)
    foreach(SOURCE ${stream_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_STREAM=1"
        )
    endforeach()
endif()

message(STATUS "Feature stream: ${stream_SOURCES}")

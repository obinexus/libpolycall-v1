# OBINexus libpolycall v2 - stream feature

file(GLOB stream_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER stream_SOURCES EXCLUDE REGEX ".*CMake.*")

if(stream_SOURCES)
    set(stream_SOURCES ${stream_SOURCES} PARENT_SCOPE)
    message(STATUS "Stream module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

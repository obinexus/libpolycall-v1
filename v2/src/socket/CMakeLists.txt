# OBINexus libpolycall v2 - socket feature

file(GLOB socket_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER socket_SOURCES EXCLUDE REGEX ".*CMake.*")

if(socket_SOURCES)
    set(socket_SOURCES ${socket_SOURCES} PARENT_SCOPE)
    message(STATUS "Socket module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

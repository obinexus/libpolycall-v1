# OBINexus libpolycall v2 - zero feature

file(GLOB zero_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER zero_SOURCES EXCLUDE REGEX ".*CMake.*")

if(zero_SOURCES)
    set(zero_SOURCES ${zero_SOURCES} PARENT_SCOPE)
    message(STATUS "Zero module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

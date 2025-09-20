# OBINexus libpolycall v2 - adapter feature

file(GLOB adapter_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER adapter_SOURCES EXCLUDE REGEX ".*CMake.*")

if(adapter_SOURCES)
    set(adapter_SOURCES ${adapter_SOURCES} PARENT_SCOPE)
    message(STATUS "Adapter module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

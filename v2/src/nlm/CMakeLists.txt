# OBINexus libpolycall v2 - NLM-Atlas feature

file(GLOB nlm_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER nlm_SOURCES EXCLUDE REGEX ".*CMake.*")
list(FILTER nlm_SOURCES EXCLUDE REGEX ".*altas.*")  # Remove typo version

if(nlm_SOURCES)
    set(nlm_SOURCES ${nlm_SOURCES} PARENT_SCOPE)
    message(STATUS "NLM module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

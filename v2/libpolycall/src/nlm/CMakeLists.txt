# Feature: nlm
# OBINexus libpolycall v2 - nlm module

# Collect sources for nlm
file(GLOB nlm_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB nlm_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/nlm/*.h")

# Export to parent scope
set(nlm_SOURCES ${nlm_SOURCES} PARENT_SCOPE)
set(nlm_HEADERS ${nlm_HEADERS} PARENT_SCOPE)

# Feature-specific definitions
if(nlm_SOURCES)
    foreach(SOURCE ${nlm_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_NLM=1"
        )
    endforeach()
endif()

message(STATUS "Feature nlm: ${nlm_SOURCES}")

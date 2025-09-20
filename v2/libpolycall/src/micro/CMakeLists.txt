# Feature: micro
# OBINexus libpolycall v2 - micro module

# Collect sources for micro
file(GLOB micro_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB micro_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/micro/*.h")

# Export to parent scope
set(micro_SOURCES ${micro_SOURCES} PARENT_SCOPE)
set(micro_HEADERS ${micro_HEADERS} PARENT_SCOPE)

# Feature-specific definitions
if(micro_SOURCES)
    foreach(SOURCE ${micro_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_MICRO=1"
        )
    endforeach()
endif()

message(STATUS "Feature micro: ${micro_SOURCES}")

# Feature: zero
# OBINexus libpolycall v2 - zero module

# Collect sources for zero
file(GLOB zero_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB zero_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/zero/*.h")

# Export to parent scope
set(zero_SOURCES ${zero_SOURCES} PARENT_SCOPE)
set(zero_HEADERS ${zero_HEADERS} PARENT_SCOPE)

# Feature-specific definitions
if(zero_SOURCES)
    foreach(SOURCE ${zero_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_ZERO=1"
        )
    endforeach()
endif()

message(STATUS "Feature zero: ${zero_SOURCES}")

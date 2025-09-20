# Feature: @FEATURE_NAME@
# Auto-generated CMakeLists.txt for feature module

# Collect sources for this feature
file(GLOB @FEATURE_NAME@_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB @FEATURE_NAME@_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/@FEATURE_NAME@/*.h")

# Export to parent scope
set(@FEATURE_NAME@_SOURCES ${@FEATURE_NAME@_SOURCES} PARENT_SCOPE)
set(@FEATURE_NAME@_HEADERS ${@FEATURE_NAME@_HEADERS} PARENT_SCOPE)

# Feature-specific compile definitions
if(@FEATURE_NAME@_SOURCES)
    foreach(SOURCE ${@FEATURE_NAME@_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_@FEATURE_NAME_UPPER@=1"
        )
    endforeach()
endif()

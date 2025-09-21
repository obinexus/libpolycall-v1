# Socket Module - Pure WebSocket Interface Layer
set(MODULE_NAME socket)
set(MODULE_VERSION 2.0.0)

# Collect sources
file(GLOB MODULE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Module headers
file(GLOB MODULE_HEADERS
    "${CMAKE_SOURCE_DIR}/include/libpolycall/${MODULE_NAME}/*.h"
)

# Object library for reuse
add_library(${MODULE_NAME}_obj OBJECT ${MODULE_SOURCES})
set_target_properties(${MODULE_NAME}_obj PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Store object files in organized structure
set_target_properties(${MODULE_NAME}_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OBJECT_OUTPUT_DIR}/${MODULE_NAME}
)

# Static library (.a)
add_library(${MODULE_NAME}_static STATIC 
    $<TARGET_OBJECTS:${MODULE_NAME}_obj>
)
set_target_properties(${MODULE_NAME}_static PROPERTIES
    OUTPUT_NAME "polycall_${MODULE_NAME}"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

# Shared library (.so/.dylib/.dll)
add_library(${MODULE_NAME}_shared SHARED 
    $<TARGET_OBJECTS:${MODULE_NAME}_obj>
)
set_target_properties(${MODULE_NAME}_shared PROPERTIES
    OUTPUT_NAME "polycall_${MODULE_NAME}"
    VERSION ${MODULE_VERSION}
    SOVERSION 2
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

# Link dependencies
target_link_libraries(${MODULE_NAME}_static Threads::Threads)
target_link_libraries(${MODULE_NAME}_shared Threads::Threads)

# Export symbols for FFI
if(UNIX AND NOT APPLE)
    set_target_properties(${MODULE_NAME}_shared PROPERTIES
        LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports.map"
    )
endif()

# Install targets
install(TARGETS ${MODULE_NAME}_static ${MODULE_NAME}_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

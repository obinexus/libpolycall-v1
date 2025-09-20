# OBINexus libpolycall v2 - micro feature
# Builds both micro.so and micro.a

file(GLOB micro_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER micro_SOURCES EXCLUDE REGEX ".*CMake.*")

if(micro_SOURCES)
    # Static library: micro.a
    add_library(micro_static STATIC ${micro_SOURCES})
    set_target_properties(micro_static PROPERTIES
        OUTPUT_NAME "micro"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    
    # Shared library: micro.so
    add_library(micro_shared SHARED ${micro_SOURCES})
    set_target_properties(micro_shared PROPERTIES
        OUTPUT_NAME "micro"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    
    # Export sources to parent
    set(micro_SOURCES ${micro_SOURCES} PARENT_SCOPE)
    message(STATUS "Micro module: ${micro_SOURCES}")
endif()

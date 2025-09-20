cmake_minimum_required(VERSION 3.10)
project(libpolycall_core)

set(CORE_SOURCES
    polycall.c
    token.c
    tokenizer.c
    context.c
    parser.c
    state_machine.c
)

# Build static library
add_library(polycall_core_static STATIC ${CORE_SOURCES})
target_include_directories(polycall_core_static 
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/internal
)

# Build shared library
add_library(polycall_core_shared SHARED ${CORE_SOURCES})
set_target_properties(polycall_core_shared PROPERTIES
    OUTPUT_NAME polycall_core
    VERSION 2.0.0
    SOVERSION 2
)

# Install targets
install(TARGETS polycall_core_static polycall_core_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Core Module - Pure Message Dispatch Layer
set(MODULE_NAME core)
set(MODULE_VERSION 2.0.0)

# Core sources - no business logic
set(CORE_SOURCES
    polycall.c
    polycall_parser.c      # Pure parsing, no rules
    polycall_protocol.c    # Protocol translation only
    state_machine.c        # State tracking, no decisions
    token.c               # Token representation only
    tokenizer.c           # Lexical analysis only
)

# Build object library
add_library(${MODULE_NAME}_obj OBJECT ${CORE_SOURCES})

# Fix missing includes
target_compile_definitions(${MODULE_NAME}_obj PRIVATE
    _GNU_SOURCE
)
target_include_directories(${MODULE_NAME}_obj PRIVATE
    ${CMAKE_SOURCE_DIR}/include/libpolycall/core
)

# Static and shared libraries
add_library(${MODULE_NAME}_static STATIC 
    $<TARGET_OBJECTS:${MODULE_NAME}_obj>
)
add_library(${MODULE_NAME}_shared SHARED 
    $<TARGET_OBJECTS:${MODULE_NAME}_obj>
)

set_target_properties(${MODULE_NAME}_static PROPERTIES
    OUTPUT_NAME "polycall_${MODULE_NAME}"
)
set_target_properties(${MODULE_NAME}_shared PROPERTIES
    OUTPUT_NAME "polycall_${MODULE_NAME}"
    VERSION ${MODULE_VERSION}
    SOVERSION 2
)

target_link_libraries(${MODULE_NAME}_static Threads::Threads)
target_link_libraries(${MODULE_NAME}_shared Threads::Threads)

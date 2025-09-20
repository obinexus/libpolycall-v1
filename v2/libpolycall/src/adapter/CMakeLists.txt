cmake_minimum_required(VERSION 3.10)
project(libpolycall_adapter)

set(ADAPTER_SOURCES
    polycall_dop_adapter.c
    polycall_dop_adapter_invoke.c
    polycall_dop_adapter_lifecycle.c
    polycall_dop_adapter_memory.c
    polycall_dop_adapter_security.c
    polycall_dop_adapter_utils.c
    polycall_dop_bridge_registry.c
)

add_library(polycall_adapter_static STATIC ${ADAPTER_SOURCES})
target_include_directories(polycall_adapter_static 
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/internal
)

add_library(polycall_adapter_shared SHARED ${ADAPTER_SOURCES})
set_target_properties(polycall_adapter_shared PROPERTIES
    OUTPUT_NAME polycall_adapter
    VERSION 2.0.0
    SOVERSION 2
)

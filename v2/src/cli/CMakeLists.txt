# OBINexus libpolycall v2 - CLI component
# Fixed: removed non-existent pid_manager.c reference

# Collect actual CLI sources
set(CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/polycall_cli.c
)

# Only add executable if sources exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/polycall_cli.c")
    add_executable(polycall_cli ${CLI_SOURCES})
    
    # Link with main library
    if(TARGET polycall_shared)
        target_link_libraries(polycall_cli polycall_shared)
    elseif(TARGET polycall_static)
        target_link_libraries(polycall_cli polycall_static)
    endif()
    
    # Set output directory
    set_target_properties(polycall_cli PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    # Installation
    install(TARGETS polycall_cli DESTINATION bin)
else()
    message(WARNING "CLI source not found, skipping CLI build")
endif()

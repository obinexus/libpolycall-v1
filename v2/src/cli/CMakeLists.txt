# CLI - Pure Interface Tool
add_executable(polycall_cli
    polycall_cli.c
    pid_manager.c
)

# Link all modules
foreach(MODULE ${POLYCALL_MODULES})
    if(TARGET ${MODULE}_shared)
        target_link_libraries(polycall_cli ${MODULE}_shared)
    endif()
endforeach()

set_target_properties(polycall_cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

install(TARGETS polycall_cli DESTINATION bin)

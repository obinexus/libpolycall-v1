# OBINexus CLI - Separate executable build
cmake_minimum_required(VERSION 3.10)

# CLI executable sources
set(CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/polycall_cli.c
    ${CMAKE_CURRENT_SOURCE_DIR}/monitor/pid_manager.c
)

# Create CLI executable (not part of library)
add_executable(polycall_cli ${CLI_SOURCES})

# Link with polycall libraries
if(TARGET polycall_shared)
    target_link_libraries(polycall_cli polycall_shared)
else()
    target_link_libraries(polycall_cli polycall_static)
endif()

# Additional libraries
target_link_libraries(polycall_cli 
    pthread
    ${CMAKE_DL_LIBS}
)

# Installation
install(TARGETS polycall_cli
    RUNTIME DESTINATION bin
)

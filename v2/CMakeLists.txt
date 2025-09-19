cmake_minimum_required(VERSION 3.10)
project(PolyCall VERSION 2.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find threads
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Collect source files from proper directories ONLY
file(GLOB CORE_SOURCES "src/core/*.c")
file(GLOB NETWORK_SOURCES "src/network/*.c")
file(GLOB PROTOCOL_SOURCES "src/protocol/*.c")
file(GLOB STATE_SOURCES "src/state/*.c")

set(ALL_SOURCES 
    ${CORE_SOURCES}
    ${NETWORK_SOURCES}
    ${PROTOCOL_SOURCES}
    ${STATE_SOURCES}
)

# Only proceed if sources exist
if(ALL_SOURCES)
    # Static library
    add_library(polycall_static STATIC ${ALL_SOURCES})
    target_link_libraries(polycall_static Threads::Threads)
    
    # Shared library
    add_library(polycall_shared SHARED ${ALL_SOURCES})
    target_link_libraries(polycall_shared Threads::Threads)
    
    # Install rules
    install(TARGETS polycall_static polycall_shared
            DESTINATION lib)
    install(DIRECTORY include/polycall 
            DESTINATION include)
else()
    message(FATAL_ERROR "No source files found in src/")
endif()

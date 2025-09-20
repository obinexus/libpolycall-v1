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

# CMakeLists.txt for dynamic module loading
function(load_module_pattern pattern)
    file(GLOB_RECURSE modules 
         "${CMAKE_SOURCE_DIR}/modules/${pattern}/*.c")
    
    foreach(module_src ${modules})
        get_filename_component(module_name ${module_src} NAME_WE)
        get_filename_component(module_dir ${module_src} DIRECTORY)
        
        # Create static library
        add_library(${module_name}_static STATIC ${module_src})
        set_target_properties(${module_name}_static PROPERTIES
            OUTPUT_NAME ${module_name}
            ARCHIVE_OUTPUT_DIRECTORY ${module_dir}
        )
        
        # Create shared library
        add_library(${module_name}_shared SHARED ${module_src})
        set_target_properties(${module_name}_shared PROPERTIES
            OUTPUT_NAME ${module_name}
            LIBRARY_OUTPUT_DIRECTORY ${module_dir}
        )
    endforeach()
endfunction()

# Load all feature-A modules
load_module_pattern("feature-A/component-subset-*/module-name-*")

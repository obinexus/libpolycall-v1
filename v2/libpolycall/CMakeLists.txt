# OBINexus libpolycall v2 - Master CMake Configuration
# Handles both modules/ and src/ hierarchies
cmake_minimum_required(VERSION 3.15)
project(libpolycall VERSION 2.0.0 LANGUAGES C)

# Global settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
if(APPLE)
    set(LIB_EXT "dylib")
    set(CMAKE_MACOSX_RPATH ON)
elseif(WIN32)
    set(LIB_EXT "dll")
else()
    set(LIB_EXT "so")
endif()

# Build options
option(BUILD_SHARED "Build shared library" ON)
option(BUILD_STATIC "Build static library" ON)
option(BUILD_MODULES "Build feature modules from modules/" ON)
option(BUILD_FEATURES "Build features from src/" ON)

# Thread support
find_package(Threads REQUIRED)

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/libpolycall
    ${CMAKE_SOURCE_DIR}/include/polycall
    ${CMAKE_SOURCE_DIR}/namespace/include
)

# Initialize source collections
set(ALL_SOURCES)
set(ALL_HEADERS)

# Function to recursively find module sources
function(collect_module_sources MODULE_DIR OUTPUT_VAR)
    set(MODULE_SOURCES)
    
    # Find all .c files recursively
    file(GLOB_RECURSE FOUND_SOURCES 
        "${MODULE_DIR}/*.c"
        "${MODULE_DIR}/*/*.c"
        "${MODULE_DIR}/*/*/*.c"
    )
    
    # Filter out empty/stub files
    foreach(SOURCE ${FOUND_SOURCES})
        if(EXISTS "${SOURCE}")
            file(SIZE "${SOURCE}" FILE_SIZE)
            if(FILE_SIZE GREATER 0)
                list(APPEND MODULE_SOURCES ${SOURCE})
            endif()
        endif()
    endforeach()
    
    set(${OUTPUT_VAR} ${MODULE_SOURCES} PARENT_SCOPE)
endfunction()

# Process modules/ directory (feature-A through feature-Z)
if(BUILD_MODULES AND EXISTS "${CMAKE_SOURCE_DIR}/modules")
    message(STATUS "=== Processing feature modules ===")
    
    # Get all feature directories
    file(GLOB FEATURE_DIRS "${CMAKE_SOURCE_DIR}/modules/feature-*")
    
    foreach(FEATURE_DIR ${FEATURE_DIRS})
        get_filename_component(FEATURE_NAME ${FEATURE_DIR} NAME)
        
        # Collect sources from all component-subsets
        collect_module_sources(${FEATURE_DIR} ${FEATURE_NAME}_SOURCES)
        
        if(${FEATURE_NAME}_SOURCES)
            message(STATUS "  ${FEATURE_NAME}: ${${FEATURE_NAME}_SOURCES}")
            list(APPEND ALL_SOURCES ${${FEATURE_NAME}_SOURCES})
            
            # Add feature-specific compile definitions
            foreach(SOURCE ${${FEATURE_NAME}_SOURCES})
                set_source_files_properties(${SOURCE} PROPERTIES
                    COMPILE_DEFINITIONS "OBINEXUS_MODULE_${FEATURE_NAME}=1"
                )
            endforeach()
        endif()
        
        # Collect corresponding headers
        file(GLOB_RECURSE ${FEATURE_NAME}_HEADERS
            "${CMAKE_SOURCE_DIR}/include/libpolycall/${FEATURE_NAME}/*.h"
        )
        list(APPEND ALL_HEADERS ${${FEATURE_NAME}_HEADERS})
    endforeach()
endif()

# Process src/ directory (implemented features)
if(BUILD_FEATURES AND EXISTS "${CMAKE_SOURCE_DIR}/src")
    message(STATUS "=== Processing src features ===")
    
    # Core features (always included)
    set(CORE_FEATURES core adapter hotwire socket micro nlm stream zero)
    
    foreach(FEATURE ${CORE_FEATURES})
        set(FEATURE_DIR "${CMAKE_SOURCE_DIR}/src/${FEATURE}")
        
        if(EXISTS ${FEATURE_DIR})
            # Check for feature-specific CMakeLists.txt
            if(EXISTS "${FEATURE_DIR}/CMakeLists.txt")
                add_subdirectory(${FEATURE_DIR})
            else()
                # Direct source collection
                file(GLOB_RECURSE FEATURE_SOURCES 
                    "${FEATURE_DIR}/*.c"
                )
                
                # Filter out CMake test files
                list(FILTER FEATURE_SOURCES EXCLUDE REGEX ".*CMakeC.*")
                
                if(FEATURE_SOURCES)
                    message(STATUS "  Feature ${FEATURE}: ${FEATURE_SOURCES}")
                    list(APPEND ALL_SOURCES ${FEATURE_SOURCES})
                    
                    # Add compile definitions
                    foreach(SOURCE ${FEATURE_SOURCES})
                        set_source_files_properties(${SOURCE} PROPERTIES
                            COMPILE_DEFINITIONS "POLYCALL_FEATURE_${FEATURE}=1"
                        )
                    endforeach()
                endif()
            endif()
            
            # Collect headers
            file(GLOB_RECURSE FEATURE_HEADERS
                "${CMAKE_SOURCE_DIR}/include/libpolycall/${FEATURE}/*.h"
                "${CMAKE_SOURCE_DIR}/include/polycall/polycall_${FEATURE}*.h"
            )
            list(APPEND ALL_HEADERS ${FEATURE_HEADERS})
        endif()
    endforeach()
endif()

# Add namespace sources
if(EXISTS "${CMAKE_SOURCE_DIR}/namespace/src")
    file(GLOB NAMESPACE_SOURCES "${CMAKE_SOURCE_DIR}/namespace/src/*.c")
    list(APPEND ALL_SOURCES ${NAMESPACE_SOURCES})
endif()

# Remove duplicates and empty entries
list(REMOVE_DUPLICATES ALL_SOURCES)
list(REMOVE_DUPLICATES ALL_HEADERS)

# Status report
message(STATUS "=== Build Configuration ===")
message(STATUS "Total sources: ${ALL_SOURCES}")
message(STATUS "Total headers: ${ALL_HEADERS}")

# Create libraries only if we have sources
if(ALL_SOURCES)
    # Static library
    if(BUILD_STATIC)
        add_library(polycall_static STATIC ${ALL_SOURCES})
        target_link_libraries(polycall_static PRIVATE Threads::Threads)
        set_target_properties(polycall_static PROPERTIES
            OUTPUT_NAME "polycall"
            PREFIX "lib"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        )
    endif()
    
    # Shared library
    if(BUILD_SHARED)
        add_library(polycall_shared SHARED ${ALL_SOURCES})
        target_link_libraries(polycall_shared PRIVATE Threads::Threads)
        set_target_properties(polycall_shared PROPERTIES
            OUTPUT_NAME "polycall"
            VERSION ${PROJECT_VERSION}
            SOVERSION 2
            PREFIX "lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
        
        # Platform-specific shared library settings
        if(APPLE)
            set_target_properties(polycall_shared PROPERTIES
                INSTALL_NAME_DIR "@rpath"
                BUILD_WITH_INSTALL_RPATH TRUE
            )
        endif()
    endif()
else()
    message(WARNING "No source files found - creating stub library")
    
    # Create minimal stub
    file(WRITE "${CMAKE_BINARY_DIR}/stub.c" 
        "int polycall_init(void) { return 0; }\n"
        "const char* polycall_version(void) { return \"2.0.0\"; }\n"
    )
    
    if(BUILD_STATIC)
        add_library(polycall_static STATIC "${CMAKE_BINARY_DIR}/stub.c")
    endif()
    if(BUILD_SHARED)
        add_library(polycall_shared SHARED "${CMAKE_BINARY_DIR}/stub.c")
    endif()
endif()

# Installation rules
install(DIRECTORY include/libpolycall DESTINATION include)
install(DIRECTORY include/polycall DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY namespace/include DESTINATION include FILES_MATCHING PATTERN "*.h")

if(TARGET polycall_static)
    install(TARGETS polycall_static
        ARCHIVE DESTINATION lib
    )
endif()

if(TARGET polycall_shared)
    install(TARGETS polycall_shared
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

# Export configuration
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/polycall-config.cmake.in"
    "${CMAKE_BINARY_DIR}/polycall-config.cmake"
    @ONLY
)

# Create pkg-config file
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/polycall.pc.in"
    "${CMAKE_BINARY_DIR}/polycall.pc"
    @ONLY
)
install(FILES "${CMAKE_BINARY_DIR}/polycall.pc" DESTINATION lib/pkgconfig)

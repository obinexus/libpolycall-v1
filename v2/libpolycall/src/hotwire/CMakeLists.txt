# Feature: hotwire
# OBINexus libpolycall v2 - hotwire module

# Collect sources for hotwire
file(GLOB hotwire_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB hotwire_HEADERS "${CMAKE_SOURCE_DIR}/include/libpolycall/hotwire/*.h")

# Export to parent scope
set(hotwire_SOURCES ${hotwire_SOURCES} PARENT_SCOPE)
set(hotwire_HEADERS ${hotwire_HEADERS} PARENT_SCOPE)

# Feature-specific definitions
if(hotwire_SOURCES)
    foreach(SOURCE ${hotwire_SOURCES})
        set_source_files_properties(${SOURCE} PROPERTIES
            COMPILE_DEFINITIONS "POLYCALL_FEATURE_HOTWIRE=1"
        )
    endforeach()
endif()

message(STATUS "Feature hotwire: ${hotwire_SOURCES}")

# CMakeLists.txt with your architecture
project(libpolycall VERSION 2.0.0)

# Core library - pure transport
add_library(polycall_core SHARED
    src/core/dispatch.c
    src/core/transport.c
)

# Language bindings
foreach(LANG IN ITEMS python node java go rust)
    add_subdirectory(bindings/${LANG})
endforeach()

# Generate SDK packages
add_custom_target(sdk_all
    DEPENDS sdk_macos sdk_linux sdk_windows
)

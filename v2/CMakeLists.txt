# v2/CMakeLists.txt.unified
cmake_minimum_required(VERSION 3.10)
project(PolyCallV2 VERSION 2.0.0)

# Thread safety requirements
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Build both libraries
add_library(polycall_static STATIC ${CORE_SOURCES})
add_library(polycall_shared SHARED ${CORE_SOURCES})

target_compile_definitions(polycall_static PRIVATE STATIC_BUILD)
target_compile_definitions(polycall_shared PRIVATE SHARED_BUILD)

# Link thread support
target_link_libraries(polycall_static Threads::Threads)
target_link_libraries(polycall_shared Threads::Threads)

# TDD targets
enable_testing()
add_test(NAME thread_safety_test COMMAND test_thread_safety)
add_test(NAME integration_test COMMAND test_integration)

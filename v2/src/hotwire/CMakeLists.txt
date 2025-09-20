# OBINexus libpolycall v2 - hotwire feature

file(GLOB hotwire_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER hotwire_SOURCES EXCLUDE REGEX ".*CMake.*")

if(hotwire_SOURCES)
    set(hotwire_SOURCES ${hotwire_SOURCES} PARENT_SCOPE)
    message(STATUS "Hotwire module: Found ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# src/xml/CMakeLists.txt
set(MODULE_NAME xml)
set(MODULE_VERSION 2.0.0)

set(XML_SOURCES
    xml_tokenizer.c      # Pure XML lexical analysis
    xml_parser.c         # DOM tree construction only
    xml_navigator.c      # Tree traversal utilities
    xml_serializer.c     # XML output generation
    config_reader.c      # Config file reader (no logic)
)

add_library(${MODULE_NAME}_obj OBJECT ${XML_SOURCES})
add_library(${MODULE_NAME}_static STATIC $<TARGET_OBJECTS:${MODULE_NAME}_obj>)
add_library(${MODULE_NAME}_shared SHARED $<TARGET_OBJECTS:${MODULE_NAME}_obj>)

set_target_properties(${MODULE_NAME}_static PROPERTIES
    OUTPUT_NAME "polycall_xml"
)
set_target_properties(${MODULE_NAME}_shared PROPERTIES
    OUTPUT_NAME "polycall_xml"
    VERSION ${MODULE_VERSION}
    SOVERSION 2
)

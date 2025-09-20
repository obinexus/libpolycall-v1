# OBINexus Module Processor
# Handles feature-X/component-subset-Y/module-name-ZZZ hierarchy

function(process_feature_modules FEATURE_LETTER)
    set(FEATURE_DIR "${CMAKE_SOURCE_DIR}/modules/feature-${FEATURE_LETTER}")
    set(MODULE_SOURCES)
    
    if(EXISTS ${FEATURE_DIR})
        # Process each component subset
        file(GLOB COMPONENT_DIRS "${FEATURE_DIR}/component-subset-*")
        
        foreach(COMPONENT_DIR ${COMPONENT_DIRS})
            get_filename_component(COMPONENT_NAME ${COMPONENT_DIR} NAME)
            
            # Find module source files
            file(GLOB MODULE_FILES 
                "${COMPONENT_DIR}/*.c"
                "${COMPONENT_DIR}/*/*.c"
            )
            
            foreach(MODULE_FILE ${MODULE_FILES})
                if(EXISTS ${MODULE_FILE})
                    list(APPEND MODULE_SOURCES ${MODULE_FILE})
                    
                    # Add module-specific definitions
                    get_filename_component(MODULE_NAME ${MODULE_FILE} NAME_WE)
                    set_source_files_properties(${MODULE_FILE} PROPERTIES
                        COMPILE_DEFINITIONS "MODULE_${MODULE_NAME}=1;FEATURE_${FEATURE_LETTER}=1"
                    )
                endif()
            endforeach()
        endforeach()
    endif()
    
    set(FEATURE_${FEATURE_LETTER}_SOURCES ${MODULE_SOURCES} PARENT_SCOPE)
endfunction()

# Process all feature modules A-Z
foreach(LETTER A B C D E F G H I J K L M N O P Q R S T U V W X Y Z)
    process_feature_modules(${LETTER})
endforeach()

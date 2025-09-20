#!/bin/bash
# Generate a module following the namespace pattern

if [ $# -ne 3 ]; then
    echo "Usage: $0 <feature-letter> <component-number> <module-number>"
    echo "Example: $0 A 1 001"
    exit 1
fi

FEATURE=$1
COMPONENT=$2
MODULE=$(printf "%03d" $3)

NAMESPACE="feature-${FEATURE}/component-subset-${COMPONENT}/module-name-${MODULE}"
MODULE_DIR="modules/${NAMESPACE}"
INCLUDE_DIR="include/libpolycall/${NAMESPACE}"

# Create directories
mkdir -p "$MODULE_DIR"
mkdir -p "$INCLUDE_DIR"

# Generate header file
cat > "${INCLUDE_DIR}.h" << EOH
#ifndef LIBPOLYCALL_FEATURE_${FEATURE}_COMPONENT_${COMPONENT}_MODULE_${MODULE}_H
#define LIBPOLYCALL_FEATURE_${FEATURE}_COMPONENT_${COMPONENT}_MODULE_${MODULE}_H

#ifdef __cplusplus
extern "C" {
#endif

// Module initialization
int module_${MODULE}_init(void);

// Module cleanup
void module_${MODULE}_cleanup(void);

// Module-specific functionality
void* module_${MODULE}_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif
EOH

# Generate source file
cat > "${MODULE_DIR}.c" << EOC
#include <libpolycall/${NAMESPACE}.h>
#include <stdlib.h>
#include <stdio.h>

static struct {
    int initialized;
    void* private_data;
} module_state = {0};

int module_${MODULE}_init(void) {
    if (module_state.initialized) {
        return 0; // Already initialized
    }
    
    printf("Initializing module: ${NAMESPACE}\n");
    module_state.private_data = calloc(1, 1024);
    module_state.initialized = 1;
    
    return 0;
}

void module_${MODULE}_cleanup(void) {
    if (!module_state.initialized) {
        return;
    }
    
    free(module_state.private_data);
    module_state.private_data = NULL;
    module_state.initialized = 0;
}

void* module_${MODULE}_get_interface(void) {
    return module_state.private_data;
}
EOC

echo "Generated module: ${NAMESPACE}"
echo "  Header: ${INCLUDE_DIR}.h"
echo "  Source: ${MODULE_DIR}.c"

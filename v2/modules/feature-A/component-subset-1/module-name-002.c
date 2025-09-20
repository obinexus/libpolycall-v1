#include <libpolycall/feature-A/component-subset-1/module-name-002.h>
#include <stdlib.h>
#include <stdio.h>

static struct {
    int initialized;
    void* private_data;
} module_state = {0};

int module_002_init(void) {
    if (module_state.initialized) {
        return 0; // Already initialized
    }
    
    printf("Initializing module: feature-A/component-subset-1/module-name-002\n");
    module_state.private_data = calloc(1, 1024);
    module_state.initialized = 1;
    
    return 0;
}

void module_002_cleanup(void) {
    if (!module_state.initialized) {
        return;
    }
    
    free(module_state.private_data);
    module_state.private_data = NULL;
    module_state.initialized = 0;
}

void* module_002_get_interface(void) {
    return module_state.private_data;
}

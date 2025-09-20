#ifndef LIBPOLYCALL_FEATURE_A_COMPONENT_1_MODULE_001_H
#define LIBPOLYCALL_FEATURE_A_COMPONENT_1_MODULE_001_H

#ifdef __cplusplus
extern "C" {
#endif

// Module initialization
int module_001_init(void);

// Module cleanup
void module_001_cleanup(void);

// Module-specific functionality
void* module_001_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif

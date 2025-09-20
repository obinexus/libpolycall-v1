#ifndef LIBPOLYCALL_FEATURE-A_MODULE-NAME-002_H
#define LIBPOLYCALL_FEATURE-A_MODULE-NAME-002_H

#include "libpolycall/core/polycall.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Module: module-name-002 in feature feature-A */

/* Module-specific types */
typedef struct {
    polycall_context_t context;
    void* module_data;
    uint32_t module_id;
} module-name-002_handle_t;

/* Module initialization */
polycall_status_t module-name-002_init(
    polycall_context_t ctx,
    module-name-002_handle_t** handle
);

/* Module operations */
polycall_status_t module-name-002_process(
    module-name-002_handle_t* handle,
    const void* input,
    void* output
);

/* Module cleanup */
void module-name-002_cleanup(module-name-002_handle_t* handle);

/* Module version */
const char* module-name-002_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* LIBPOLYCALL_FEATURE-A_MODULE-NAME-002_H */

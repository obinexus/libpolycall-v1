#ifndef LIBPOLYCALL_FEATURE-B_MODULE-NAME-003_H
#define LIBPOLYCALL_FEATURE-B_MODULE-NAME-003_H

#include "libpolycall/core/polycall.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Module: module-name-003 in feature feature-B */

/* Module-specific types */
typedef struct {
    polycall_context_t context;
    void* module_data;
    uint32_t module_id;
} module-name-003_handle_t;

/* Module initialization */
polycall_status_t module-name-003_init(
    polycall_context_t ctx,
    module-name-003_handle_t** handle
);

/* Module operations */
polycall_status_t module-name-003_process(
    module-name-003_handle_t* handle,
    const void* input,
    void* output
);

/* Module cleanup */
void module-name-003_cleanup(module-name-003_handle_t* handle);

/* Module version */
const char* module-name-003_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* LIBPOLYCALL_FEATURE-B_MODULE-NAME-003_H */

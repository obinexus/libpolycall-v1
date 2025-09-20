#ifndef LIBPOLYCALL_FEATURE-H_MODULE-NAME-008_H
#define LIBPOLYCALL_FEATURE-H_MODULE-NAME-008_H

#include "libpolycall/core/polycall.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Module: module-name-008 in feature feature-H */

/* Module-specific types */
typedef struct {
    polycall_context_t context;
    void* module_data;
    uint32_t module_id;
} module-name-008_handle_t;

/* Module initialization */
polycall_status_t module-name-008_init(
    polycall_context_t ctx,
    module-name-008_handle_t** handle
);

/* Module operations */
polycall_status_t module-name-008_process(
    module-name-008_handle_t* handle,
    const void* input,
    void* output
);

/* Module cleanup */
void module-name-008_cleanup(module-name-008_handle_t* handle);

/* Module version */
const char* module-name-008_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* LIBPOLYCALL_FEATURE-H_MODULE-NAME-008_H */

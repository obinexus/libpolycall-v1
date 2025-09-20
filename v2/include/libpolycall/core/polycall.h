#ifndef LIBPOLYCALL_CORE_POLYCALL_H
#define LIBPOLYCALL_CORE_POLYCALL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration for opaque pointer */
struct polycall_context;

/* Opaque context type - this is a POINTER */
typedef struct polycall_context* polycall_context_t;

/* Status codes */
typedef enum {
    POLYCALL_SUCCESS = 0,
    POLYCALL_ERROR_INVALID_PARAMETERS = -1,
    POLYCALL_ERROR_OUT_OF_MEMORY = -2,
    POLYCALL_ERROR_NOT_INITIALIZED = -3,
    POLYCALL_ERROR_ALREADY_INITIALIZED = -4,
    POLYCALL_ERROR_INVALID_STATE = -5,
    POLYCALL_ERROR_TIMEOUT = -6,
    POLYCALL_ERROR_UNKNOWN = -99
} polycall_status_t;

/* Configuration structure */
typedef struct {
    unsigned int flags;
    size_t memory_pool_size;
    void* user_data;
} polycall_config_t;

/* Core API functions */
polycall_status_t polycall_init_with_config(
    polycall_context_t* ctx, 
    const polycall_config_t* config
);

void polycall_cleanup(polycall_context_t ctx);
const char* polycall_get_version(void);
const char* polycall_get_last_error(polycall_context_t ctx);

#ifdef __cplusplus
}
#endif

#endif /* LIBPOLYCALL_CORE_POLYCALL_H */

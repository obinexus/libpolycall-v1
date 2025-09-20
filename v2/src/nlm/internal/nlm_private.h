#ifndef LIBPOLYCALL_NLM_PRIVATE_H
#define LIBPOLYCALL_NLM_PRIVATE_H

// Private definitions for nlm module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct nlm_internal {
    void* private_data;
    uint32_t flags;
} nlm_internal_t;

#endif // LIBPOLYCALL_NLM_PRIVATE_H

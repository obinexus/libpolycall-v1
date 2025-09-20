#ifndef LIBPOLYCALL_MICRO_PRIVATE_H
#define LIBPOLYCALL_MICRO_PRIVATE_H

// Private definitions for micro module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct micro_internal {
    void* private_data;
    uint32_t flags;
} micro_internal_t;

#endif // LIBPOLYCALL_MICRO_PRIVATE_H

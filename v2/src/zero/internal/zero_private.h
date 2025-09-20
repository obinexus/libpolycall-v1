#ifndef LIBPOLYCALL_ZERO_PRIVATE_H
#define LIBPOLYCALL_ZERO_PRIVATE_H

// Private definitions for zero module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct zero_internal {
    void* private_data;
    uint32_t flags;
} zero_internal_t;

#endif // LIBPOLYCALL_ZERO_PRIVATE_H

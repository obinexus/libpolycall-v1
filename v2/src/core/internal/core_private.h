#ifndef LIBPOLYCALL_CORE_PRIVATE_H
#define LIBPOLYCALL_CORE_PRIVATE_H

// Private definitions for core module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct core_internal {
    void* private_data;
    uint32_t flags;
} core_internal_t;

#endif // LIBPOLYCALL_CORE_PRIVATE_H

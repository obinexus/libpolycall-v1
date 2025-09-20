#ifndef LIBPOLYCALL_ADAPTER_PRIVATE_H
#define LIBPOLYCALL_ADAPTER_PRIVATE_H

// Private definitions for adapter module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct adapter_internal {
    void* private_data;
    uint32_t flags;
} adapter_internal_t;

#endif // LIBPOLYCALL_ADAPTER_PRIVATE_H

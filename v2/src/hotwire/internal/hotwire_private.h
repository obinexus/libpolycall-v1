#ifndef LIBPOLYCALL_HOTWIRE_PRIVATE_H
#define LIBPOLYCALL_HOTWIRE_PRIVATE_H

// Private definitions for hotwire module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct hotwire_internal {
    void* private_data;
    uint32_t flags;
} hotwire_internal_t;

#endif // LIBPOLYCALL_HOTWIRE_PRIVATE_H

#ifndef LIBPOLYCALL_STREAM_PRIVATE_H
#define LIBPOLYCALL_STREAM_PRIVATE_H

// Private definitions for stream module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct stream_internal {
    void* private_data;
    uint32_t flags;
} stream_internal_t;

#endif // LIBPOLYCALL_STREAM_PRIVATE_H

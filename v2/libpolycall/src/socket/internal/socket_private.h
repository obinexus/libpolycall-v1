#ifndef LIBPOLYCALL_SOCKET_PRIVATE_H
#define LIBPOLYCALL_SOCKET_PRIVATE_H

// Private definitions for socket module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct socket_internal {
    void* private_data;
    uint32_t flags;
} socket_internal_t;

#endif // LIBPOLYCALL_SOCKET_PRIVATE_H

#ifndef LIBPOLYCALL_CORE_POLYCALL_H
#define LIBPOLYCALL_CORE_POLYCALL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// OBINexus libpolycall v2 Core API
#define POLYCALL_VERSION "2.0.0"

// Core context structure
typedef struct {
    void* data;
    size_t size;
    uint32_t flags;
    void* user_data;
} polycall_context_t;

// Core initialization
int polycall_init(polycall_context_t* ctx);
void polycall_cleanup(polycall_context_t* ctx);

// Core operations
int polycall_execute(polycall_context_t* ctx, const char* command);
int polycall_process(polycall_context_t* ctx);

#endif // LIBPOLYCALL_CORE_POLYCALL_H

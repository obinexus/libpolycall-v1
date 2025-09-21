#ifndef POLYCALL_DOP_ADAPTER_H
#define POLYCALL_DOP_ADAPTER_H

#include <stddef.h>
#include <stdint.h>

// DOP adapter types
typedef struct {
    void* data;
    size_t size;
    uint32_t flags;
} polycall_dop_adapter_t;

// Adapter functions
void* polycall_dop_adapter_create(size_t size);
void polycall_dop_adapter_destroy(void* adapter);

#endif // POLYCALL_DOP_ADAPTER_H

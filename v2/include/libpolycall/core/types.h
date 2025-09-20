#ifndef POLYCALL_CORE_TYPES_H
#define POLYCALL_CORE_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Forward declarations */
struct polycall_context;
typedef struct polycall_context* polycall_context_t;

/* Basic types used across the system */
typedef uint32_t polycall_id_t;
typedef uint32_t polycall_flags_t;
typedef int32_t polycall_result_t;

/* Memory management types */
typedef struct {
    void* ptr;
    size_t size;
    size_t used;
} polycall_buffer_t;

/* Adapter types */
typedef struct {
    polycall_id_t adapter_id;
    const char* name;
    void* handle;
    void* user_data;
} polycall_adapter_t;

/* Bridge types */
typedef struct {
    polycall_id_t bridge_id;
    polycall_adapter_t* source;
    polycall_adapter_t* target;
} polycall_bridge_t;

/* Function pointer types */
typedef int (*polycall_init_fn)(void* ctx);
typedef int (*polycall_process_fn)(void* ctx, void* data);
typedef void (*polycall_cleanup_fn)(void* ctx);

#endif /* POLYCALL_CORE_TYPES_H */

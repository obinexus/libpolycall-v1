#ifndef LIBPOLYCALL_MODULE_SCHEMA_H
#define LIBPOLYCALL_MODULE_SCHEMA_H

#include <stdint.h>

/* Module schema version */
#define MODULE_SCHEMA_VERSION "1.0.0"

/* Module types */
typedef enum {
    MODULE_TYPE_CORE = 0x0001,
    MODULE_TYPE_ADAPTER = 0x0002,
    MODULE_TYPE_NETWORK = 0x0004,
    MODULE_TYPE_CRYPTO = 0x0008,
    MODULE_TYPE_STORAGE = 0x0010,
    MODULE_TYPE_COMPUTE = 0x0020,
    MODULE_TYPE_CUSTOM = 0x8000
} module_type_t;

/* Module capability flags */
typedef enum {
    MODULE_CAP_ASYNC = 0x0001,
    MODULE_CAP_THREAD_SAFE = 0x0002,
    MODULE_CAP_STATELESS = 0x0004,
    MODULE_CAP_REALTIME = 0x0008,
    MODULE_CAP_STREAMING = 0x0010
} module_capability_t;

/* Module metadata */
typedef struct {
    char name[64];
    char version[32];
    module_type_t type;
    uint32_t capabilities;
    uint32_t required_memory;
    uint32_t max_instances;
} module_metadata_t;

/* Module registry entry */
typedef struct {
    uint32_t module_id;
    module_metadata_t metadata;
    void* (*create_func)(void);
    void (*destroy_func)(void*);
    int (*init_func)(void*, void*);
    int (*process_func)(void*, const void*, void*);
} module_registry_entry_t;

/* Module loader functions */
int module_schema_register(const module_registry_entry_t* entry);
int module_schema_unregister(uint32_t module_id);
const module_registry_entry_t* module_schema_get(uint32_t module_id);
int module_schema_enumerate(module_registry_entry_t** entries, size_t* count);

#endif /* LIBPOLYCALL_MODULE_SCHEMA_H */

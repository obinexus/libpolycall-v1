#ifndef POLYCALL_MICRO_H
#define POLYCALL_MICRO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "libpolycall/core/types.h"
#include "libpolycall/core/polycall.h"

#define POLYCALL_MICRO_MAX_COMMANDS 256
#define POLYCALL_MICRO_MAX_SERVICES 32
#define POLYCALL_MICRO_BUFFER_SIZE 4096

typedef enum {
    POLYCALL_MICRO_SUCCESS = 0,
    POLYCALL_MICRO_ERROR_INIT = -1,
    POLYCALL_MICRO_ERROR_SERVICE = -2,
    POLYCALL_MICRO_ERROR_COMMAND = -3,
    POLYCALL_MICRO_ERROR_PROTOCOL = -4,
    POLYCALL_MICRO_ERROR_MEMORY = -5
} PolycallMicroStatus;

typedef struct {
    uint32_t id;
    uint32_t type;
    uint32_t flags;
    uint64_t timestamp;
    size_t payload_size;
    uint8_t payload[POLYCALL_MICRO_BUFFER_SIZE];
} PolycallCommand;

typedef struct {
    PolycallCommand commands[POLYCALL_MICRO_MAX_COMMANDS];
    uint32_t count;
    uint32_t capacity;
} PolycallCommandArray;

typedef struct {
    uint32_t id;
    uint32_t state;
    uint32_t flags;
    uint64_t last_update;
    PolycallCommandArray command_queue;
    uint8_t data_buffer[POLYCALL_MICRO_BUFFER_SIZE];
} PolycallServiceState;

typedef struct {
    PolycallServiceState services[POLYCALL_MICRO_MAX_SERVICES];
    uint32_t count;
    uint32_t active_mask;
    uint64_t last_gc;
} PolycallServiceArray;

typedef void* polycall_sm_t;

typedef enum {
    POLYCALL_SM_SUCCESS = 0,
    POLYCALL_SM_ERROR = -1
} polycall_sm_status_t;

typedef struct {
    void* context;
    NetworkEndpoint* endpoint;
    uint32_t flags;
    uint32_t max_message_size;
    uint32_t timeout_ms;
    void* user_data;
} polycall_protocol_context_t;

typedef struct {
    uint32_t flags;
    size_t max_message_size;
    uint32_t timeout_ms;
    void* user_data;
} polycall_protocol_config_t;

typedef struct {
    PolycallServiceArray service_array;
    polycall_sm_t state_machine;
    polycall_protocol_context_t protocol_ctx;
    uint32_t flags;
    uint64_t startup_time;
} PolycallMicroContext;

typedef void (*PolycallTransform)(PolycallCommand* cmd);
typedef bool (*PolycallPredicate)(const PolycallCommand* cmd);
typedef void (*PolycallOperation)(PolycallServiceState* state);

typedef struct {
    PolycallTransform* transforms;
    uint32_t transform_count;
} PolycallTransformChain;

PolycallMicroStatus polycall_micro_init(
    PolycallMicroContext* ctx,
    const polycall_config_t* config
);
void polycall_micro_cleanup(PolycallMicroContext* ctx);
PolycallMicroStatus polycall_micro_create_service(
    PolycallMicroContext* ctx,
    uint32_t service_id,
    uint32_t flags
);
PolycallMicroStatus polycall_micro_destroy_service(
    PolycallMicroContext* ctx,
    uint32_t service_id
);
PolycallMicroStatus polycall_micro_update_service_state(
    PolycallMicroContext* ctx,
    uint32_t service_id,
    uint32_t new_state
);
PolycallMicroStatus polycall_micro_transform_command(
    PolycallCommand* cmd,
    const PolycallTransformChain* chain
);
PolycallMicroStatus polycall_micro_filter_commands(
    PolycallCommandArray* commands,
    PolycallPredicate predicate
);
PolycallMicroStatus polycall_micro_process_commands(
    PolycallServiceState* service,
    PolycallOperation operation
);
PolycallMicroStatus polycall_micro_batch_process(
    PolycallMicroContext* ctx,
    uint32_t service_id,
    const PolycallCommandArray* commands
);
PolycallMicroStatus polycall_micro_collect_garbage(PolycallMicroContext* ctx);
const char* polycall_micro_status_string(PolycallMicroStatus status);
uint32_t polycall_micro_get_active_services(const PolycallMicroContext* ctx);
PolycallTransformChain polycall_micro_create_transform_chain(
    PolycallTransform* transforms,
    uint32_t count
);
void polycall_micro_destroy_transform_chain(PolycallTransformChain* chain);

polycall_sm_status_t polycall_sm_create_with_integrity(
    void* user_data,
    polycall_sm_t* sm,
    void* integrity_check
);
void polycall_sm_destroy(polycall_sm_t sm);
bool polycall_protocol_init(
    polycall_protocol_context_t* ctx,
    void* user_data,
    NetworkEndpoint* endpoint,
    const polycall_protocol_config_t* config
);
void polycall_protocol_cleanup(polycall_protocol_context_t* ctx);

#endif /* POLYCALL_MICRO_H */

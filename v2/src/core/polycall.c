#include "libpolycall/core/polycall.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define POLYCALL_VERSION "2.0.0"
#define MAX_ERROR_LENGTH 256

/* Internal context structure - hidden from API users */
struct polycall_context {
    char last_error[MAX_ERROR_LENGTH];
    void* user_data;
    size_t memory_pool_size;
    unsigned int flags;
    bool is_initialized;
};

/* Static helper functions */
static void set_error(polycall_context_t ctx, const char* error) {
    if (ctx && error) {
        strncpy(ctx->last_error, error, MAX_ERROR_LENGTH - 1);
        ctx->last_error[MAX_ERROR_LENGTH - 1] = '\0';
    }
}

/* API Implementation */

polycall_status_t polycall_init_with_config(
    polycall_context_t* ctx, 
    const polycall_config_t* config
) {
    if (!ctx) {
        return POLYCALL_ERROR_INVALID_PARAMETERS;
    }

    /* Allocate context */
    struct polycall_context* new_ctx = calloc(1, sizeof(struct polycall_context));
    if (!new_ctx) {
        return POLYCALL_ERROR_OUT_OF_MEMORY;
    }

    /* Initialize with defaults */
    new_ctx->memory_pool_size = 1024 * 1024; /* 1MB default */
    new_ctx->flags = 0;

    /* Apply configuration if provided */
    if (config) {
        new_ctx->flags = config->flags;
        if (config->memory_pool_size > 0) {
            new_ctx->memory_pool_size = config->memory_pool_size;
        }
        new_ctx->user_data = config->user_data;
    }

    /* Mark as initialized */
    new_ctx->is_initialized = true;
    set_error(new_ctx, NULL); /* Clear any errors - NOW USING set_error */

    *ctx = new_ctx;
    return POLYCALL_SUCCESS;
}

void polycall_cleanup(polycall_context_t ctx) {
    if (ctx) {
        if (!ctx->is_initialized) {
            set_error(ctx, "Context not initialized"); /* Using set_error */
            return;
        }
        ctx->is_initialized = false;
        free(ctx);
    }
}

const char* polycall_get_version(void) {
    return POLYCALL_VERSION;
}

const char* polycall_get_last_error(polycall_context_t ctx) {
    if (!ctx) {
        return "Invalid context";
    }
    return ctx->last_error[0] ? ctx->last_error : "No error";
}

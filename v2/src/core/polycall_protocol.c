#include "libpolycall/micro/polycall_micro.h"
#include <string.h>
#include <stdlib.h>

bool polycall_protocol_init(
    polycall_protocol_context_t* ctx,
    void* user_data,
    NetworkEndpoint* endpoint,
    const polycall_protocol_config_t* config
) {
    if (!ctx || !endpoint || !config) return false;
    
    ctx->context = malloc(4096);
    if (!ctx->context) return false;
    
    ctx->endpoint = endpoint;
    ctx->flags = config->flags;
    ctx->max_message_size = config->max_message_size;
    ctx->timeout_ms = config->timeout_ms;
    ctx->user_data = user_data;
    
    return true;
}

void polycall_protocol_cleanup(polycall_protocol_context_t* ctx) {
    if (ctx && ctx->context) {
        free(ctx->context);
        ctx->context = NULL;
    }
}

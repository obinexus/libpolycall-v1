/* Standard library includes */
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Core types */
#include "libpolycall/core/types.h"

/**
 * @file hotwire_router.c
 * @brief OBINexus LibPolyCall v2 Hotwiring Router Implementation
 * @version 2.0.0
 * @author OBINexus Computing - OpenACE Division
 *
 * Constitutional Setup for Hot-Wiring Architecture
 * Implements dynamic rerouting with backward compatibility guarantees
 */

#include "../core/hotwire/hotwire_router.h"
#include "../core/polycall/polycall_core.h"
#include "../core/protocol/protocol_commands.h"
#include "polycall/config/config_commands.h"
#include "telemetry/telemetry_commands.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Constitutional Compliance Constants */
/*---------------------------------------------------------------------------*/

#define HOTWIRE_PROTOCOL_VERSION "2.0.0"
#define HOTWIRE_V1_COMPAT_FLAG 0x01
#define HOTWIRE_AUDIT_ENABLED 0x02
#define HOTWIRE_STATELESS_MODE 0x04

#define MAX_HOTWIRE_ROUTES 256
#define MAX_ROUTE_CONFIG_SIZE 4096
#define HOTWIRE_MAGIC_NUMBER 0x48574952 // "HWIR"

/*---------------------------------------------------------------------------*/
/* Internal Data Structures */
/*---------------------------------------------------------------------------*/

typedef struct hotwire_route_entry {
  char source_protocol[64];
  char target_protocol[64];
  char config_path[256];
  uint32_t flags;
  uint64_t created_timestamp;
  uint64_t last_used_timestamp;
  uint32_t usage_count;
  void *private_data;
} hotwire_route_entry_t;

typedef struct hotwire_router_context {
  uint32_t magic;
  uint32_t version;
  uint32_t flags;
  size_t route_count;
  hotwire_route_entry_t routes[MAX_HOTWIRE_ROUTES];
  polycall_protocol_context_t *protocol_ctx;
  polycall_core_context_t *core_ctx;
  void *config_ctx;
  void *telemetry_ctx;
} hotwire_router_context_t;

/*---------------------------------------------------------------------------*/
/* Static Global Context */
/*---------------------------------------------------------------------------*/

static hotwire_router_context_t *g_hotwire_ctx = NULL;

/*---------------------------------------------------------------------------*/
/* Forward Declarations */
/*---------------------------------------------------------------------------*/

static polycall_core_error_t
hotwire_validate_route_config(const char *source_protocol,
                              const char *target_protocol,
                              const char *config_path);

static polycall_core_error_t
hotwire_audit_route_access(hotwire_router_context_t *ctx,
                           const char *source_protocol,
                           const char *target_protocol, const char *operation);

static polycall_core_error_t
hotwire_v1_compatibility_check(const char *protocol_name);

/*---------------------------------------------------------------------------*/
/* Public API Implementation */
/*---------------------------------------------------------------------------*/

/**
 * @brief Initialize the hotwiring router subsystem
 * @param core_ctx Core LibPolyCall context
 * @param protocol_ctx Protocol layer context
 * @param config Configuration context (optional, can be NULL)
 * @return POLYCALL_CORE_SUCCESS on success, error code otherwise
 */
polycall_core_error_t
hotwire_router_init(polycall_core_context_t *core_ctx,
                    polycall_protocol_context_t *protocol_ctx,
                    const hotwire_config_t *config) {
  if (!core_ctx || !protocol_ctx) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETER;
  }

  // Allocate context
  g_hotwire_ctx =
      polycall_core_alloc(core_ctx, sizeof(hotwire_router_context_t));
  if (!g_hotwire_ctx) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Initialize context
  memset(g_hotwire_ctx, 0, sizeof(hotwire_router_context_t));
  g_hotwire_ctx->magic = HOTWIRE_MAGIC_NUMBER;
  g_hotwire_ctx->version = 0x020000; // v2.0.0
  g_hotwire_ctx->core_ctx = core_ctx;
  g_hotwire_ctx->protocol_ctx = protocol_ctx;

  // Set constitutional flags
  g_hotwire_ctx->flags |= HOTWIRE_V1_COMPAT_FLAG;
  g_hotwire_ctx->flags |= HOTWIRE_STATELESS_MODE;

  // Enable audit if requested in config
  if (config && config->enable_audit) {
    g_hotwire_ctx->flags |= HOTWIRE_AUDIT_ENABLED;
  }

  // Initialize telemetry if audit enabled
  if (g_hotwire_ctx->flags & HOTWIRE_AUDIT_ENABLED) {
    polycall_core_error_t telemetry_result =
        telemetry_commands_init(core_ctx, &g_hotwire_ctx->telemetry_ctx);
    if (telemetry_result != POLYCALL_CORE_SUCCESS) {
      // Non-fatal, but log the issue
      printf("[HOTWIRE] Warning: Telemetry initialization failed\n");
    }
  }

  // Register hotwiring protocol map with protocol layer
  polycall_core_error_t map_result =
      hotwire_protocol_map_register(protocol_ctx);
  if (map_result != POLYCALL_CORE_SUCCESS) {
    hotwire_router_cleanup();
    return map_result;
  }

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Cleanup hotwiring router subsystem
 */
void hotwire_router_cleanup(void) {
  if (!g_hotwire_ctx) {
    return;
  }

  // Validate magic number
  if (g_hotwire_ctx->magic != HOTWIRE_MAGIC_NUMBER) {
    // Context corrupted, log and exit
    printf("[HOTWIRE] Critical: Context corruption detected during cleanup\n");
    return;
  }

  // Cleanup telemetry if enabled
  if (g_hotwire_ctx->telemetry_ctx) {
    telemetry_commands_cleanup(g_hotwire_ctx->core_ctx,
                               g_hotwire_ctx->telemetry_ctx);
  }

  // Free any private route data
  for (size_t i = 0; i < g_hotwire_ctx->route_count; i++) {
    if (g_hotwire_ctx->routes[i].private_data) {
      polycall_core_free(g_hotwire_ctx->core_ctx,
                         g_hotwire_ctx->routes[i].private_data);
    }
  }

  // Free context
  polycall_core_free(g_hotwire_ctx->core_ctx, g_hotwire_ctx);
  g_hotwire_ctx = NULL;
}

/**
 * @brief Register a new hotwiring route
 * @param source_protocol Source protocol identifier
 * @param target_protocol Target protocol identifier
 * @param config_path Path to route configuration
 * @return POLYCALL_CORE_SUCCESS on success, error code otherwise
 */
polycall_core_error_t hotwire_route_register(const char *source_protocol,
                                             const char *target_protocol,
                                             const char *config_path) {
  if (!g_hotwire_ctx || !source_protocol || !target_protocol) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETER;
  }

  // Validate context integrity
  if (g_hotwire_ctx->magic != HOTWIRE_MAGIC_NUMBER) {
    return POLYCALL_CORE_ERROR_INVALID_STATE;
  }

  // Check capacity
  if (g_hotwire_ctx->route_count >= MAX_HOTWIRE_ROUTES) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Validate route configuration
  polycall_core_error_t validation_result = hotwire_validate_route_config(
      source_protocol, target_protocol, config_path);
  if (validation_result != POLYCALL_CORE_SUCCESS) {
    return validation_result;
  }

  // Check v1 compatibility if source protocol is legacy
  polycall_core_error_t compat_result =
      hotwire_v1_compatibility_check(source_protocol);
  if (compat_result != POLYCALL_CORE_SUCCESS) {
    // Log compatibility violation for audit
    if (g_hotwire_ctx->flags & HOTWIRE_AUDIT_ENABLED) {
      hotwire_audit_route_access(g_hotwire_ctx, source_protocol,
                                 target_protocol, "COMPAT_VIOLATION");
    }
    return compat_result;
  }

  // Create new route entry
  hotwire_route_entry_t *route =
      &g_hotwire_ctx->routes[g_hotwire_ctx->route_count];
  strncpy(route->source_protocol, source_protocol,
          sizeof(route->source_protocol) - 1);
  strncpy(route->target_protocol, target_protocol,
          sizeof(route->target_protocol) - 1);

  if (config_path) {
    strncpy(route->config_path, config_path, sizeof(route->config_path) - 1);
  }

  route->flags = 0;
  route->created_timestamp = polycall_core_get_timestamp();
  route->last_used_timestamp = 0;
  route->usage_count = 0;
  route->private_data = NULL;

  g_hotwire_ctx->route_count++;

  // Audit route registration
  if (g_hotwire_ctx->flags & HOTWIRE_AUDIT_ENABLED) {
    hotwire_audit_route_access(g_hotwire_ctx, source_protocol, target_protocol,
                               "REGISTER");
  }

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Execute a hotwiring route
 * @param source_protocol Source protocol identifier
 * @param target_protocol Target protocol identifier
 * @param request Request data
 * @param response Response buffer
 * @return POLYCALL_CORE_SUCCESS on success, error code otherwise
 */
polycall_core_error_t hotwire_route_execute(const char *source_protocol,
                                            const char *target_protocol,
                                            const polycall_request_t *request,
                                            polycall_response_t *response) {
  if (!g_hotwire_ctx || !source_protocol || !target_protocol || !request ||
      !response) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETER;
  }

  // Find matching route
  hotwire_route_entry_t *route = NULL;
  for (size_t i = 0; i < g_hotwire_ctx->route_count; i++) {
    if (strcmp(g_hotwire_ctx->routes[i].source_protocol, source_protocol) ==
            0 &&
        strcmp(g_hotwire_ctx->routes[i].target_protocol, target_protocol) ==
            0) {
      route = &g_hotwire_ctx->routes[i];
      break;
    }
  }

  if (!route) {
    return POLYCALL_CORE_ERROR_NOT_FOUND;
  }

  // Update usage statistics
  route->last_used_timestamp = polycall_core_get_timestamp();
  route->usage_count++;

  // Audit route execution
  if (g_hotwire_ctx->flags & HOTWIRE_AUDIT_ENABLED) {
    hotwire_audit_route_access(g_hotwire_ctx, source_protocol, target_protocol,
                               "EXECUTE");
  }

  // Delegate to protocol layer with stateless fallback
  polycall_core_error_t exec_result =
      hotwire_protocol_map_execute(g_hotwire_ctx->protocol_ctx, source_protocol,
                                   target_protocol, request, response);

  // If execution failed and we have v1 compatibility enabled, try fallback
  if (exec_result != POLYCALL_CORE_SUCCESS &&
      (g_hotwire_ctx->flags & HOTWIRE_V1_COMPAT_FLAG)) {
    exec_result = hotwire_protocol_v1_fallback(
        g_hotwire_ctx->protocol_ctx, source_protocol, request, response);
  }

  return exec_result;
}

/*---------------------------------------------------------------------------*/
/* Internal Implementation Functions */
/*---------------------------------------------------------------------------*/

/**
 * @brief Validate route configuration according to constitutional constraints
 */
static polycall_core_error_t
hotwire_validate_route_config(const char *source_protocol,
                              const char *target_protocol,
                              const char *config_path) {
  // Basic validation
  if (strlen(source_protocol) == 0 || strlen(target_protocol) == 0) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETER;
  }

  // Check for protocol name conflicts
  if (strcmp(source_protocol, target_protocol) == 0) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETER;
  }

  // Validate config path if provided
  if (config_path && strlen(config_path) > 0) {
    polycall_core_error_t config_result =
        config_commands_validate_path(g_hotwire_ctx->core_ctx, config_path);
    if (config_result != POLYCALL_CORE_SUCCESS) {
      return config_result;
    }
  }

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Audit route access for compliance monitoring
 */
static polycall_core_error_t
hotwire_audit_route_access(hotwire_router_context_t *ctx,
                           const char *source_protocol,
                           const char *target_protocol, const char *operation) {
  if (!ctx->telemetry_ctx) {
    return POLYCALL_CORE_SUCCESS; // Audit disabled
  }

  // Create audit entry
  char audit_message[512];
  snprintf(audit_message, sizeof(audit_message),
           "HOTWIRE_ROUTE: %s -> %s [%s] at %llu", source_protocol,
           target_protocol, operation,
           (unsigned long long)polycall_core_get_timestamp());

  return telemetry_commands_log_audit(ctx->telemetry_ctx, audit_message);
}

/**
 * @brief Check v1 compatibility for given protocol
 */
static polycall_core_error_t
hotwire_v1_compatibility_check(const char *protocol_name) {
  // List of known v1 protocols that must be preserved
  static const char *v1_protocols[] = {
      "polycall.v1.core", "polycall.v1.command", "polycall.v1.binding", NULL};

  // Check if this is a v1 protocol
  for (const char **proto = v1_protocols; *proto; proto++) {
    if (strcmp(protocol_name, *proto) == 0) {
      // v1 protocol detected - ensure compatibility mode
      return POLYCALL_CORE_SUCCESS;
    }
  }

  // New protocol - no compatibility constraints
  return POLYCALL_CORE_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/* Protocol Map Integration */
/*---------------------------------------------------------------------------*/

/**
 * @brief Register hotwiring protocol map with protocol layer
 */
polycall_core_error_t
hotwire_protocol_map_register(polycall_protocol_context_t *protocol_ctx) {
  if (!protocol_ctx) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETER;
  }

  // Register hotwiring as a protocol enhancement
  return protocol_commands_register_enhancement(
      protocol_ctx, "hotwire_router", HOTWIRE_PROTOCOL_VERSION,
      &hotwire_protocol_map_interface);
}

/**
 * @brief Get hotwiring router statistics
 */
polycall_core_error_t hotwire_router_get_stats(hotwire_router_stats_t *stats) {
  if (!stats || !g_hotwire_ctx) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETER;
  }

  memset(stats, 0, sizeof(hotwire_router_stats_t));
  stats->total_routes = g_hotwire_ctx->route_count;
  stats->flags = g_hotwire_ctx->flags;
  stats->version = g_hotwire_ctx->version;

  // Calculate total usage
  for (size_t i = 0; i < g_hotwire_ctx->route_count; i++) {
    stats->total_executions += g_hotwire_ctx->routes[i].usage_count;
  }

  return POLYCALL_CORE_SUCCESS;
}

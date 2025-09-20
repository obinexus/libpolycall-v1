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
 * @file polycall_dop_adapter.c
 * @brief DOP (Data-Oriented Programming) Adapter Core Implementation
 *
 * LibPolyCall DOP Adapter Framework - Core Implementation
 * OBINexus Computing - Aegis Project Technical Infrastructure
 *
 * Implements universal cross-language micro-component adapter framework
 * with strict Zero Trust security enforcement and IoC compliance.
 *
 * @version 1.0.0
 * @date 2025-06-09
 */

#include "polycall/core/dop/polycall_dop_adapter.h"
#include "polycall/core/polycall_core.h"
#include "polycall/core/polycall_memory.h"
#include "polycall/core/polycall_security.h"

#include <assert.h>
#include <string.h>
#include <time.h>

/* ====================================================================
 * Internal Function Declarations
 * ==================================================================== */

static polycall_dop_error_t dop_adapter_initialize_security(
    polycall_dop_adapter_context_t *adapter_ctx,
    const polycall_dop_security_policy_t *default_policy);

static polycall_dop_error_t
dop_adapter_initialize_memory(polycall_dop_adapter_context_t *adapter_ctx);

static polycall_dop_error_t
dop_adapter_validate_context(const polycall_dop_adapter_context_t *adapter_ctx);

static polycall_dop_error_t
dop_component_validate_transition(polycall_dop_component_t *component,
                                  polycall_dop_component_state_t new_state);

static void dop_audit_log_internal(polycall_dop_adapter_context_t *adapter_ctx,
                                   polycall_dop_audit_event_type_t event_type,
                                   const char *component_id,
                                   const char *method_name,
                                   polycall_dop_error_t error_code,
                                   const char *details);

static uint64_t dop_get_timestamp_ns(void);

/* ====================================================================
 * Primary API Implementation - Adapter Lifecycle
 * ==================================================================== */

polycall_dop_error_t polycall_dop_adapter_initialize(
    polycall_core_context_t *core_ctx,
    polycall_dop_adapter_context_t **adapter_ctx,
    const polycall_dop_security_policy_t *default_policy) {
  polycall_dop_error_t result = POLYCALL_DOP_SUCCESS;
  polycall_dop_adapter_context_t *ctx = NULL;

  // Zero Trust validation - verify all inputs
  if (!core_ctx || !adapter_ctx || !default_policy) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate core context is properly initialized
  if (!polycall_core_is_initialized(core_ctx)) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  // Allocate adapter context using core memory manager
  ctx = (polycall_dop_adapter_context_t *)polycall_memory_allocate(
      core_ctx, sizeof(polycall_dop_adapter_context_t));
  if (!ctx) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Initialize context structure
  memset(ctx, 0, sizeof(polycall_dop_adapter_context_t));
  ctx->core_ctx = core_ctx;
  ctx->is_initialized = false;
  ctx->zero_trust_enabled = true; // Default to maximum security

  // Copy default security policy
  memcpy(&ctx->default_policy, default_policy,
         sizeof(polycall_dop_security_policy_t));

  // Initialize component registry
  ctx->component_capacity = 32; // Initial capacity
  ctx->components = (polycall_dop_component_t **)polycall_memory_allocate(
      core_ctx, sizeof(polycall_dop_component_t *) * ctx->component_capacity);
  if (!ctx->components) {
    polycall_memory_free(core_ctx, ctx);
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }
  memset(ctx->components, 0,
         sizeof(polycall_dop_component_t *) * ctx->component_capacity);

  // Initialize bridge registry
  ctx->bridges = (polycall_dop_bridge_t **)polycall_memory_allocate(
      core_ctx,
      sizeof(polycall_dop_bridge_t *) * 16 // Support up to 16 languages
  );
  if (!ctx->bridges) {
    polycall_memory_free(core_ctx, ctx->components);
    polycall_memory_free(core_ctx, ctx);
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }
  memset(ctx->bridges, 0, sizeof(polycall_dop_bridge_t *) * 16);

  // Initialize audit buffer
  ctx->audit_buffer_size = 1024; // Buffer for 1024 events
  ctx->audit_buffer = (polycall_dop_audit_event_t *)polycall_memory_allocate(
      core_ctx, sizeof(polycall_dop_audit_event_t) * ctx->audit_buffer_size);
  if (!ctx->audit_buffer) {
    polycall_memory_free(core_ctx, ctx->bridges);
    polycall_memory_free(core_ctx, ctx->components);
    polycall_memory_free(core_ctx, ctx);
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }
  memset(ctx->audit_buffer, 0,
         sizeof(polycall_dop_audit_event_t) * ctx->audit_buffer_size);

  // Initialize security subsystem
  result = dop_adapter_initialize_security(ctx, default_policy);
  if (result != POLYCALL_DOP_SUCCESS) {
    polycall_memory_free(core_ctx, ctx->audit_buffer);
    polycall_memory_free(core_ctx, ctx->bridges);
    polycall_memory_free(core_ctx, ctx->components);
    polycall_memory_free(core_ctx, ctx);
    return result;
  }

  // Initialize memory management subsystem
  result = dop_adapter_initialize_memory(ctx);
  if (result != POLYCALL_DOP_SUCCESS) {
    // Cleanup security context (implementation in security.c)
    polycall_memory_free(core_ctx, ctx->audit_buffer);
    polycall_memory_free(core_ctx, ctx->bridges);
    polycall_memory_free(core_ctx, ctx->components);
    polycall_memory_free(core_ctx, ctx);
    return result;
  }

  // Mark as initialized
  ctx->is_initialized = true;

  // Log initialization event
  dop_audit_log_internal(ctx, POLYCALL_DOP_AUDIT_COMPONENT_CREATED,
                         "dop_adapter", "initialize", POLYCALL_DOP_SUCCESS,
                         "DOP Adapter successfully initialized");

  *adapter_ctx = ctx;
  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_adapter_cleanup(polycall_dop_adapter_context_t *adapter_ctx) {
  if (!adapter_ctx) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_error_t validation_result =
      dop_adapter_validate_context(adapter_ctx);
  if (validation_result != POLYCALL_DOP_SUCCESS) {
    return validation_result;
  }

  polycall_core_context_t *core_ctx = adapter_ctx->core_ctx;

  // Log cleanup initiation
  dop_audit_log_internal(adapter_ctx, POLYCALL_DOP_AUDIT_COMPONENT_DESTROYED,
                         "dop_adapter", "cleanup", POLYCALL_DOP_SUCCESS,
                         "DOP Adapter cleanup initiated");

  // Cleanup all registered components
  for (size_t i = 0; i < adapter_ctx->component_count; i++) {
    if (adapter_ctx->components[i]) {
      polycall_dop_component_unregister(adapter_ctx,
                                        adapter_ctx->components[i]);
    }
  }

  // Free memory regions
  if (adapter_ctx->memory) {
    // Memory manager cleanup (implementation in memory.c)
    polycall_memory_free(core_ctx, adapter_ctx->memory);
  }

  // Free security context
  if (adapter_ctx->security) {
    // Security context cleanup (implementation in security.c)
    polycall_memory_free(core_ctx, adapter_ctx->security);
  }

  // Free audit buffer
  if (adapter_ctx->audit_buffer) {
    polycall_memory_free(core_ctx, adapter_ctx->audit_buffer);
  }

  // Free bridge registry
  if (adapter_ctx->bridges) {
    polycall_memory_free(core_ctx, adapter_ctx->bridges);
  }

  // Free component registry
  if (adapter_ctx->components) {
    polycall_memory_free(core_ctx, adapter_ctx->components);
  }

  // Mark as uninitialized
  adapter_ctx->is_initialized = false;

  // Free the context itself
  polycall_memory_free(core_ctx, adapter_ctx);

  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Component Registration and Management
 * ==================================================================== */

polycall_dop_error_t
polycall_dop_component_register(polycall_dop_adapter_context_t *adapter_ctx,
                                const polycall_dop_component_config_t *config,
                                polycall_dop_component_t **component) {
  if (!adapter_ctx || !config || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_error_t validation_result =
      dop_adapter_validate_context(adapter_ctx);
  if (validation_result != POLYCALL_DOP_SUCCESS) {
    return validation_result;
  }

  // Validate component configuration
  polycall_dop_error_t config_result =
      polycall_dop_component_config_validate(config);
  if (config_result != POLYCALL_DOP_SUCCESS) {
    return config_result;
  }

  // Check for duplicate component ID
  polycall_dop_component_t *existing = NULL;
  if (polycall_dop_component_find(adapter_ctx, config->component_id,
                                  &existing) == POLYCALL_DOP_SUCCESS) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER; // Component already exists
  }

  // Expand component registry if needed
  if (adapter_ctx->component_count >= adapter_ctx->component_capacity) {
    size_t new_capacity = adapter_ctx->component_capacity * 2;
    polycall_dop_component_t **new_components =
        (polycall_dop_component_t **)polycall_memory_allocate(
            adapter_ctx->core_ctx,
            sizeof(polycall_dop_component_t *) * new_capacity);
    if (!new_components) {
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }

    // Copy existing components
    memcpy(new_components, adapter_ctx->components,
           sizeof(polycall_dop_component_t *) *
               adapter_ctx->component_capacity);
    memset(new_components + adapter_ctx->component_capacity, 0,
           sizeof(polycall_dop_component_t *) *
               (new_capacity - adapter_ctx->component_capacity));

    polycall_memory_free(adapter_ctx->core_ctx, adapter_ctx->components);
    adapter_ctx->components = new_components;
    adapter_ctx->component_capacity = new_capacity;
  }

  // Allocate component structure
  polycall_dop_component_t *comp =
      (polycall_dop_component_t *)polycall_memory_allocate(
          adapter_ctx->core_ctx, sizeof(polycall_dop_component_t));
  if (!comp) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Initialize component structure
  memset(comp, 0, sizeof(polycall_dop_component_t));

  // Copy configuration data with bounds checking
  strncpy(comp->component_id, config->component_id,
          POLYCALL_DOP_MAX_COMPONENT_ID_LENGTH - 1);
  comp->component_id[POLYCALL_DOP_MAX_COMPONENT_ID_LENGTH - 1] = '\0';

  strncpy(comp->component_name, config->component_name,
          POLYCALL_DOP_MAX_COMPONENT_NAME_LENGTH - 1);
  comp->component_name[POLYCALL_DOP_MAX_COMPONENT_NAME_LENGTH - 1] = '\0';

  strncpy(comp->version, config->version, POLYCALL_DOP_MAX_VERSION_LENGTH - 1);
  comp->version[POLYCALL_DOP_MAX_VERSION_LENGTH - 1] = '\0';

  comp->state = POLYCALL_DOP_COMPONENT_INITIALIZING;
  comp->language = config->language;
  memcpy(&comp->security_policy, &config->security_policy,
         sizeof(polycall_dop_security_policy_t));

  // Copy method signatures
  if (config->method_count > 0 && config->methods) {
    comp->methods = (polycall_dop_method_signature_t *)polycall_memory_allocate(
        adapter_ctx->core_ctx,
        sizeof(polycall_dop_method_signature_t) * config->method_count);
    if (!comp->methods) {
      polycall_memory_free(adapter_ctx->core_ctx, comp);
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }

    memcpy(comp->methods, config->methods,
           sizeof(polycall_dop_method_signature_t) * config->method_count);
    comp->method_count = config->method_count;
  }

  // Initialize memory regions array
  comp->memory_regions = NULL;
  comp->region_count = 0;

  // Initialize statistics
  comp->invocation_count = 0;
  comp->total_execution_time_ns = 0;
  comp->total_memory_allocated = 0;
  comp->security_violations = 0;

  // Find and assign language bridge
  polycall_dop_bridge_get(adapter_ctx, config->language, &comp->bridge);

  // Register component in adapter
  adapter_ctx->components[adapter_ctx->component_count] = comp;
  adapter_ctx->component_count++;

  // Transition to ready state
  comp->state = POLYCALL_DOP_COMPONENT_READY;

  // Log registration event
  dop_audit_log_internal(adapter_ctx, POLYCALL_DOP_AUDIT_COMPONENT_CREATED,
                         comp->component_id, NULL, POLYCALL_DOP_SUCCESS,
                         "Component successfully registered");

  *component = comp;
  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_component_unregister(polycall_dop_adapter_context_t *adapter_ctx,
                                  polycall_dop_component_t *component) {
  if (!adapter_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_error_t validation_result =
      dop_adapter_validate_context(adapter_ctx);
  if (validation_result != POLYCALL_DOP_SUCCESS) {
    return validation_result;
  }

  // Find component in registry
  size_t component_index = SIZE_MAX;
  for (size_t i = 0; i < adapter_ctx->component_count; i++) {
    if (adapter_ctx->components[i] == component) {
      component_index = i;
      break;
    }
  }

  if (component_index == SIZE_MAX) {
    return POLYCALL_DOP_ERROR_COMPONENT_NOT_FOUND;
  }

  // Validate state transition to cleanup
  polycall_dop_error_t transition_result = dop_component_validate_transition(
      component, POLYCALL_DOP_COMPONENT_CLEANUP);
  if (transition_result != POLYCALL_DOP_SUCCESS) {
    return transition_result;
  }

  component->state = POLYCALL_DOP_COMPONENT_CLEANUP;

  // Log destruction event
  dop_audit_log_internal(adapter_ctx, POLYCALL_DOP_AUDIT_COMPONENT_DESTROYED,
                         component->component_id, NULL, POLYCALL_DOP_SUCCESS,
                         "Component unregistration initiated");

  // Free allocated memory regions
  if (component->memory_regions) {
    for (size_t i = 0; i < component->region_count; i++) {
      // Memory region cleanup (implementation in memory.c)
      polycall_memory_free(adapter_ctx->core_ctx,
                           component->memory_regions[i].base_address);
    }
    polycall_memory_free(adapter_ctx->core_ctx, component->memory_regions);
  }

  // Free method signatures
  if (component->methods) {
    polycall_memory_free(adapter_ctx->core_ctx, component->methods);
  }

  // Remove from component registry
  for (size_t i = component_index; i < adapter_ctx->component_count - 1; i++) {
    adapter_ctx->components[i] = adapter_ctx->components[i + 1];
  }
  adapter_ctx->component_count--;
  adapter_ctx->components[adapter_ctx->component_count] = NULL;

  // Mark as destroyed
  component->state = POLYCALL_DOP_COMPONENT_DESTROYED;

  // Free component structure
  polycall_memory_free(adapter_ctx->core_ctx, component);

  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_component_find(polycall_dop_adapter_context_t *adapter_ctx,
                            const char *component_id,
                            polycall_dop_component_t **component) {
  if (!adapter_ctx || !component_id || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_error_t validation_result =
      dop_adapter_validate_context(adapter_ctx);
  if (validation_result != POLYCALL_DOP_SUCCESS) {
    return validation_result;
  }

  // Search component registry
  for (size_t i = 0; i < adapter_ctx->component_count; i++) {
    if (adapter_ctx->components[i] &&
        strcmp(adapter_ctx->components[i]->component_id, component_id) == 0) {
      *component = adapter_ctx->components[i];
      return POLYCALL_DOP_SUCCESS;
    }
  }

  *component = NULL;
  return POLYCALL_DOP_ERROR_COMPONENT_NOT_FOUND;
}

/* ====================================================================
 * Internal Helper Functions
 * ==================================================================== */

static polycall_dop_error_t dop_adapter_initialize_security(
    polycall_dop_adapter_context_t *adapter_ctx,
    const polycall_dop_security_policy_t *default_policy) {
  // Security context initialization (detailed implementation in security.c)
  adapter_ctx->security =
      (polycall_dop_security_context_t *)polycall_memory_allocate(
          adapter_ctx->core_ctx, sizeof(polycall_dop_security_context_t));

  if (!adapter_ctx->security) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Initialize security context with default policy
  memset(adapter_ctx->security, 0, sizeof(polycall_dop_security_context_t));

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_adapter_initialize_memory(polycall_dop_adapter_context_t *adapter_ctx) {
  // Memory manager initialization (detailed implementation in memory.c)
  adapter_ctx->memory =
      (polycall_dop_memory_manager_t *)polycall_memory_allocate(
          adapter_ctx->core_ctx, sizeof(polycall_dop_memory_manager_t));

  if (!adapter_ctx->memory) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Initialize memory manager
  memset(adapter_ctx->memory, 0, sizeof(polycall_dop_memory_manager_t));

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_adapter_validate_context(
    const polycall_dop_adapter_context_t *adapter_ctx) {
  if (!adapter_ctx) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!adapter_ctx->is_initialized) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  if (!adapter_ctx->core_ctx) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_component_validate_transition(polycall_dop_component_t *component,
                                  polycall_dop_component_state_t new_state) {
  if (!component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Define valid state transitions
  switch (component->state) {
  case POLYCALL_DOP_COMPONENT_UNINITIALIZED:
    if (new_state != POLYCALL_DOP_COMPONENT_INITIALIZING) {
      return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
    }
    break;

  case POLYCALL_DOP_COMPONENT_INITIALIZING:
    if (new_state != POLYCALL_DOP_COMPONENT_READY &&
        new_state != POLYCALL_DOP_COMPONENT_ERROR) {
      return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
    }
    break;

  case POLYCALL_DOP_COMPONENT_READY:
    if (new_state != POLYCALL_DOP_COMPONENT_EXECUTING &&
        new_state != POLYCALL_DOP_COMPONENT_SUSPENDED &&
        new_state != POLYCALL_DOP_COMPONENT_CLEANUP &&
        new_state != POLYCALL_DOP_COMPONENT_ERROR) {
      return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
    }
    break;

  case POLYCALL_DOP_COMPONENT_EXECUTING:
    if (new_state != POLYCALL_DOP_COMPONENT_READY &&
        new_state != POLYCALL_DOP_COMPONENT_ERROR) {
      return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
    }
    break;

  case POLYCALL_DOP_COMPONENT_SUSPENDED:
    if (new_state != POLYCALL_DOP_COMPONENT_READY &&
        new_state != POLYCALL_DOP_COMPONENT_CLEANUP &&
        new_state != POLYCALL_DOP_COMPONENT_ERROR) {
      return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
    }
    break;

  case POLYCALL_DOP_COMPONENT_ERROR:
    if (new_state != POLYCALL_DOP_COMPONENT_READY &&
        new_state != POLYCALL_DOP_COMPONENT_CLEANUP) {
      return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
    }
    break;

  case POLYCALL_DOP_COMPONENT_CLEANUP:
    if (new_state != POLYCALL_DOP_COMPONENT_DESTROYED) {
      return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
    }
    break;

  case POLYCALL_DOP_COMPONENT_DESTROYED:
    // No transitions allowed from destroyed state
    return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;

  default:
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  return POLYCALL_DOP_SUCCESS;
}

static void dop_audit_log_internal(polycall_dop_adapter_context_t *adapter_ctx,
                                   polycall_dop_audit_event_type_t event_type,
                                   const char *component_id,
                                   const char *method_name,
                                   polycall_dop_error_t error_code,
                                   const char *details) {
  if (!adapter_ctx || !adapter_ctx->audit_buffer) {
    return; // Silent failure for audit logging
  }

  // Check if audit buffer is full
  if (adapter_ctx->audit_buffer_count >= adapter_ctx->audit_buffer_size) {
    // Circular buffer - overwrite oldest entry
    adapter_ctx->audit_buffer_count = 0;
  }

  polycall_dop_audit_event_t *event =
      &adapter_ctx->audit_buffer[adapter_ctx->audit_buffer_count];

  event->event_type = event_type;
  event->timestamp_ns = dop_get_timestamp_ns();
  event->component_id = component_id;
  event->method_name = method_name;
  event->error_code = error_code;
  event->details = details;
  event->context_data = NULL;
  event->context_size = 0;

  adapter_ctx->audit_buffer_count++;
}

static uint64_t dop_get_timestamp_ns(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
  }
  return 0; // Fallback for systems without high-resolution timer
}

/* ====================================================================
 * Utility Function Implementations
 * ==================================================================== */

const char *polycall_dop_error_string(polycall_dop_error_t error_code) {
  switch (error_code) {
  case POLYCALL_DOP_SUCCESS:
    return "Success";
  case POLYCALL_DOP_ERROR_INVALID_PARAMETER:
    return "Invalid parameter";
  case POLYCALL_DOP_ERROR_INVALID_STATE:
    return "Invalid state";
  case POLYCALL_DOP_ERROR_MEMORY_ALLOCATION:
    return "Memory allocation failed";
  case POLYCALL_DOP_ERROR_SECURITY_VIOLATION:
    return "Security violation";
  case POLYCALL_DOP_ERROR_PERMISSION_DENIED:
    return "Permission denied";
  case POLYCALL_DOP_ERROR_COMPONENT_NOT_FOUND:
    return "Component not found";
  case POLYCALL_DOP_ERROR_BRIDGE_UNAVAILABLE:
    return "Language bridge unavailable";
  case POLYCALL_DOP_ERROR_ISOLATION_BREACH:
    return "Memory isolation breach";
  case POLYCALL_DOP_ERROR_INVOKE_FAILED:
    return "Component invocation failed";
  case POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION:
    return "Invalid lifecycle transition";
  case POLYCALL_DOP_ERROR_CONFIGURATION_INVALID:
    return "Invalid configuration";
  case POLYCALL_DOP_ERROR_TIMEOUT:
    return "Operation timeout";
  case POLYCALL_DOP_ERROR_NOT_IMPLEMENTED:
    return "Feature not implemented";
  case POLYCALL_DOP_ERROR_UNKNOWN:
  default:
    return "Unknown error";
  }
}

const char *
polycall_dop_component_state_string(polycall_dop_component_state_t state) {
  switch (state) {
  case POLYCALL_DOP_COMPONENT_UNINITIALIZED:
    return "Uninitialized";
  case POLYCALL_DOP_COMPONENT_INITIALIZING:
    return "Initializing";
  case POLYCALL_DOP_COMPONENT_READY:
    return "Ready";
  case POLYCALL_DOP_COMPONENT_EXECUTING:
    return "Executing";
  case POLYCALL_DOP_COMPONENT_SUSPENDED:
    return "Suspended";
  case POLYCALL_DOP_COMPONENT_ERROR:
    return "Error";
  case POLYCALL_DOP_COMPONENT_CLEANUP:
    return "Cleanup";
  case POLYCALL_DOP_COMPONENT_DESTROYED:
    return "Destroyed";
  default:
    return "Unknown";
  }
}

const char *polycall_dop_language_string(polycall_dop_language_t language) {
  switch (language) {
  case POLYCALL_DOP_LANGUAGE_C:
    return "C/C++";
  case POLYCALL_DOP_LANGUAGE_JAVASCRIPT:
    return "JavaScript";
  case POLYCALL_DOP_LANGUAGE_PYTHON:
    return "Python";
  case POLYCALL_DOP_LANGUAGE_JVM:
    return "JVM";
  case POLYCALL_DOP_LANGUAGE_WASM:
    return "WebAssembly";
  case POLYCALL_DOP_LANGUAGE_UNKNOWN:
  default:
    return "Unknown";
  }
}

polycall_dop_error_t polycall_dop_component_config_validate(
    const polycall_dop_component_config_t *config) {
  if (!config) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate component ID
  if (!config->component_id || strlen(config->component_id) == 0 ||
      strlen(config->component_id) >= POLYCALL_DOP_MAX_COMPONENT_ID_LENGTH) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate component name
  if (!config->component_name || strlen(config->component_name) == 0 ||
      strlen(config->component_name) >=
          POLYCALL_DOP_MAX_COMPONENT_NAME_LENGTH) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate version
  if (!config->version || strlen(config->version) == 0 ||
      strlen(config->version) >= POLYCALL_DOP_MAX_VERSION_LENGTH) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate language
  if (config->language >= POLYCALL_DOP_LANGUAGE_UNKNOWN) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate method signatures if provided
  if (config->method_count > 0) {
    if (!config->methods) {
      return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
    }

    if (config->method_count > POLYCALL_DOP_MAX_METHODS_PER_COMPONENT) {
      return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
    }

    for (size_t i = 0; i < config->method_count; i++) {
      const polycall_dop_method_signature_t *method = &config->methods[i];

      if (!method->method_name || strlen(method->method_name) == 0 ||
          strlen(method->method_name) >= POLYCALL_DOP_MAX_METHOD_NAME_LENGTH) {
        return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
      }

      if (method->parameter_count > POLYCALL_DOP_MAX_PARAMETERS) {
        return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
      }

      if (method->parameter_count > 0 && !method->parameter_types) {
        return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
      }
    }
  }

  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t polycall_dop_security_policy_create_default(
    polycall_dop_isolation_level_t isolation_level,
    polycall_dop_security_policy_t *policy) {
  if (!policy) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  memset(policy, 0, sizeof(polycall_dop_security_policy_t));

  policy->isolation_level = isolation_level;
  policy->max_execution_time_ms = POLYCALL_DOP_DEFAULT_TIMEOUT_MS;
  policy->audit_enabled = true;
  policy->stack_protection_enabled = true;
  policy->heap_protection_enabled = true;

  // Set permissions based on isolation level
  switch (isolation_level) {
  case POLYCALL_DOP_ISOLATION_NONE:
    policy->allowed_permissions = POLYCALL_DOP_PERMISSION_ALL;
    policy->max_memory_usage = UINT32_MAX;
    break;

  case POLYCALL_DOP_ISOLATION_BASIC:
    policy->allowed_permissions = POLYCALL_DOP_PERMISSION_MEMORY_READ |
                                  POLYCALL_DOP_PERMISSION_MEMORY_WRITE |
                                  POLYCALL_DOP_PERMISSION_INVOKE_LOCAL;
    policy->max_memory_usage = 1024 * 1024; // 1MB
    break;

  case POLYCALL_DOP_ISOLATION_STANDARD:
    policy->allowed_permissions = POLYCALL_DOP_PERMISSION_MEMORY_READ |
                                  POLYCALL_DOP_PERMISSION_INVOKE_LOCAL;
    policy->max_memory_usage = 512 * 1024; // 512KB
    break;

  case POLYCALL_DOP_ISOLATION_STRICT:
    policy->allowed_permissions = POLYCALL_DOP_PERMISSION_MEMORY_READ;
    policy->max_memory_usage = 256 * 1024; // 256KB
    break;

  case POLYCALL_DOP_ISOLATION_PARANOID:
    policy->allowed_permissions = POLYCALL_DOP_PERMISSION_NONE;
    policy->max_memory_usage = 128 * 1024; // 128KB
    break;
  }

  policy->denied_permissions = ~policy->allowed_permissions;

  return POLYCALL_DOP_SUCCESS;
}

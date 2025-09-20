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
 * @file polycall_dop_adapter_lifecycle.c
 * @brief DOP Adapter Component Lifecycle Management
 *
 * LibPolyCall DOP Adapter Framework - Component Lifecycle Engine
 * OBINexus Computing - Aegis Project Technical Infrastructure
 *
 * Implements comprehensive component lifecycle management with state
 * validation, transition controls, and proper resource cleanup following IoC
 * principles. Critical for maintaining system stability in banking app
 * deployments.
 *
 * @version 1.0.0
 * @date 2025-06-09
 */

#include "polycall/core/dop/polycall_dop_adapter.h"
#include "polycall/core/polycall_core.h"
#include "polycall/core/polycall_memory.h"

#include <assert.h>
#include <string.h>
#include <time.h>

/* ====================================================================
 * Lifecycle State Machine Definition
 * ==================================================================== */

/**
 * @brief State transition matrix for component lifecycle
 *
 * Defines valid state transitions following strict lifecycle rules.
 * Each row represents current state, columns represent target states.
 * true = transition allowed, false = transition forbidden.
 */
static const bool LIFECYCLE_TRANSITION_MATRIX[8][8] = {
    /*                      UNINIT INIT READY EXEC SUSP ERR  CLEAN DEST */
    /* UNINITIALIZED */ {false, true, false, false, false, false, false, false},
    /* INITIALIZING */ {false, false, true, false, false, true, false, false},
    /* READY */ {false, false, false, true, true, true, true, false},
    /* EXECUTING */ {false, false, true, false, false, true, false, false},
    /* SUSPENDED */ {false, false, true, false, false, true, true, false},
    /* ERROR */ {false, false, true, false, false, false, true, false},
    /* CLEANUP */ {false, false, false, false, false, false, false, true},
    /* DESTROYED */ {false, false, false, false, false, false, false, false}};

/* ====================================================================
 * Internal Function Declarations
 * ==================================================================== */

static polycall_dop_error_t
dop_lifecycle_validate_transition(polycall_dop_component_state_t current_state,
                                  polycall_dop_component_state_t target_state);

static polycall_dop_error_t dop_lifecycle_execute_initialization(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_component_config_t *config);

static polycall_dop_error_t
dop_lifecycle_execute_cleanup(polycall_dop_adapter_context_t *adapter_ctx,
                              polycall_dop_component_t *component);

static polycall_dop_error_t dop_lifecycle_setup_runtime_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_component_config_t *config);

static polycall_dop_error_t dop_lifecycle_cleanup_runtime_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component);

static polycall_dop_error_t dop_lifecycle_validate_dependencies(
    polycall_dop_adapter_context_t *adapter_ctx,
    const polycall_dop_component_config_t *config);

static void dop_lifecycle_log_state_transition(
    polycall_dop_adapter_context_t *adapter_ctx, const char *component_id,
    polycall_dop_component_state_t from_state,
    polycall_dop_component_state_t to_state, polycall_dop_error_t result);

static uint64_t dop_lifecycle_get_timestamp_ns(void);

/* ====================================================================
 * Public Lifecycle API Implementation
 * ==================================================================== */

/**
 * @brief Initialize component with full lifecycle management
 */
polycall_dop_error_t polycall_dop_component_initialize(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_component_config_t *config) {
  if (!adapter_ctx || !component || !config) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_error_t result = POLYCALL_DOP_SUCCESS;
  polycall_dop_component_state_t initial_state = component->state;

  // Validate initial state transition
  result = dop_lifecycle_validate_transition(
      component->state, POLYCALL_DOP_COMPONENT_INITIALIZING);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  // Transition to initializing state
  component->state = POLYCALL_DOP_COMPONENT_INITIALIZING;
  dop_lifecycle_log_state_transition(
      adapter_ctx, component->component_id, initial_state,
      POLYCALL_DOP_COMPONENT_INITIALIZING, POLYCALL_DOP_SUCCESS);

  // Validate dependencies are available
  result = dop_lifecycle_validate_dependencies(adapter_ctx, config);
  if (result != POLYCALL_DOP_SUCCESS) {
    component->state = POLYCALL_DOP_COMPONENT_ERROR;
    dop_lifecycle_log_state_transition(adapter_ctx, component->component_id,
                                       POLYCALL_DOP_COMPONENT_INITIALIZING,
                                       POLYCALL_DOP_COMPONENT_ERROR, result);
    return result;
  }

  // Setup runtime context based on language
  result = dop_lifecycle_setup_runtime_context(adapter_ctx, component, config);
  if (result != POLYCALL_DOP_SUCCESS) {
    component->state = POLYCALL_DOP_COMPONENT_ERROR;
    dop_lifecycle_log_state_transition(adapter_ctx, component->component_id,
                                       POLYCALL_DOP_COMPONENT_INITIALIZING,
                                       POLYCALL_DOP_COMPONENT_ERROR, result);
    return result;
  }

  // Execute component-specific initialization
  result = dop_lifecycle_execute_initialization(adapter_ctx, component, config);
  if (result != POLYCALL_DOP_SUCCESS) {
    // Cleanup runtime context on failure
    dop_lifecycle_cleanup_runtime_context(adapter_ctx, component);
    component->state = POLYCALL_DOP_COMPONENT_ERROR;
    dop_lifecycle_log_state_transition(adapter_ctx, component->component_id,
                                       POLYCALL_DOP_COMPONENT_INITIALIZING,
                                       POLYCALL_DOP_COMPONENT_ERROR, result);
    return result;
  }

  // Transition to ready state
  component->state = POLYCALL_DOP_COMPONENT_READY;
  dop_lifecycle_log_state_transition(
      adapter_ctx, component->component_id, POLYCALL_DOP_COMPONENT_INITIALIZING,
      POLYCALL_DOP_COMPONENT_READY, POLYCALL_DOP_SUCCESS);

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Suspend component execution
 */
polycall_dop_error_t
polycall_dop_component_suspend(polycall_dop_adapter_context_t *adapter_ctx,
                               polycall_dop_component_t *component) {
  if (!adapter_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_component_state_t current_state = component->state;

  // Validate state transition
  polycall_dop_error_t result = dop_lifecycle_validate_transition(
      current_state, POLYCALL_DOP_COMPONENT_SUSPENDED);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  // Only allow suspension from READY state
  if (current_state != POLYCALL_DOP_COMPONENT_READY) {
    return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
  }

  // Transition to suspended state
  component->state = POLYCALL_DOP_COMPONENT_SUSPENDED;

  dop_lifecycle_log_state_transition(
      adapter_ctx, component->component_id, current_state,
      POLYCALL_DOP_COMPONENT_SUSPENDED, POLYCALL_DOP_SUCCESS);

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Resume component from suspended state
 */
polycall_dop_error_t
polycall_dop_component_resume(polycall_dop_adapter_context_t *adapter_ctx,
                              polycall_dop_component_t *component) {
  if (!adapter_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_component_state_t current_state = component->state;

  // Validate state transition
  polycall_dop_error_t result = dop_lifecycle_validate_transition(
      current_state, POLYCALL_DOP_COMPONENT_READY);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  // Only allow resume from SUSPENDED state
  if (current_state != POLYCALL_DOP_COMPONENT_SUSPENDED) {
    return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
  }

  // Transition to ready state
  component->state = POLYCALL_DOP_COMPONENT_READY;

  dop_lifecycle_log_state_transition(
      adapter_ctx, component->component_id, current_state,
      POLYCALL_DOP_COMPONENT_READY, POLYCALL_DOP_SUCCESS);

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Reset component from error state
 */
polycall_dop_error_t
polycall_dop_component_reset(polycall_dop_adapter_context_t *adapter_ctx,
                             polycall_dop_component_t *component) {
  if (!adapter_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  polycall_dop_component_state_t current_state = component->state;

  // Only allow reset from ERROR state
  if (current_state != POLYCALL_DOP_COMPONENT_ERROR) {
    return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
  }

  // Validate state transition
  polycall_dop_error_t result = dop_lifecycle_validate_transition(
      current_state, POLYCALL_DOP_COMPONENT_READY);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  // Reset component state and clear error indicators
  component->state = POLYCALL_DOP_COMPONENT_READY;

  // Could reset error counters here if needed
  // component->security_violations = 0;  // Optional: reset violation count

  dop_lifecycle_log_state_transition(
      adapter_ctx, component->component_id, current_state,
      POLYCALL_DOP_COMPONENT_READY, POLYCALL_DOP_SUCCESS);

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Get current component state
 */
polycall_dop_component_state_t
polycall_dop_component_get_state(const polycall_dop_component_t *component) {
  if (!component) {
    return POLYCALL_DOP_COMPONENT_UNINITIALIZED;
  }

  return component->state;
}

/**
 * @brief Check if component can accept invocations
 */
bool polycall_dop_component_is_ready(
    const polycall_dop_component_t *component) {
  if (!component) {
    return false;
  }

  return component->state == POLYCALL_DOP_COMPONENT_READY;
}

/**
 * @brief Check if state transition is valid
 */
bool polycall_dop_component_can_transition(
    const polycall_dop_component_t *component,
    polycall_dop_component_state_t target_state) {
  if (!component) {
    return false;
  }

  return dop_lifecycle_validate_transition(component->state, target_state) ==
         POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Internal Function Implementations
 * ==================================================================== */

static polycall_dop_error_t
dop_lifecycle_validate_transition(polycall_dop_component_state_t current_state,
                                  polycall_dop_component_state_t target_state) {
  // Validate state values are in range
  if (current_state < 0 || current_state >= 8 || target_state < 0 ||
      target_state >= 8) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Check transition matrix
  if (!LIFECYCLE_TRANSITION_MATRIX[current_state][target_state]) {
    return POLYCALL_DOP_ERROR_LIFECYCLE_VIOLATION;
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_lifecycle_execute_initialization(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_component_config_t *config) {
  if (!adapter_ctx || !component || !config) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Call component-specific initialization callback if provided
  if (component->init_callback) {
    polycall_dop_error_t result =
        component->init_callback(component, config, component->user_data);
    if (result != POLYCALL_DOP_SUCCESS) {
      return result;
    }
  }

  // Initialize component statistics
  component->invocation_count = 0;
  component->total_execution_time_ns = 0;
  component->total_memory_allocated = 0;
  component->security_violations = 0;

  // Log initialization completion
  polycall_dop_audit_event_t audit_event = {
      .event_type = POLYCALL_DOP_AUDIT_COMPONENT_CREATED,
      .timestamp_ns = dop_lifecycle_get_timestamp_ns(),
      .component_id = component->component_id,
      .method_name = "initialize",
      .error_code = POLYCALL_DOP_SUCCESS,
      .details = "Component initialization completed successfully",
      .context_data = NULL,
      .context_size = 0};
  polycall_dop_audit_log(adapter_ctx, &audit_event);

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_lifecycle_execute_cleanup(polycall_dop_adapter_context_t *adapter_ctx,
                              polycall_dop_component_t *component) {
  if (!adapter_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Call component-specific cleanup callback if provided
  if (component->cleanup_callback) {
    polycall_dop_error_t result =
        component->cleanup_callback(component, component->user_data);
    if (result != POLYCALL_DOP_SUCCESS) {
      // Log cleanup error but continue with destruction
      polycall_dop_audit_event_t audit_event = {
          .event_type = POLYCALL_DOP_AUDIT_COMPONENT_DESTROYED,
          .timestamp_ns = dop_lifecycle_get_timestamp_ns(),
          .component_id = component->component_id,
          .method_name = "cleanup",
          .error_code = result,
          .details = "Component cleanup callback failed",
          .context_data = NULL,
          .context_size = 0};
      polycall_dop_audit_log(adapter_ctx, &audit_event);
    }
  }

  // Cleanup runtime context
  dop_lifecycle_cleanup_runtime_context(adapter_ctx, component);

  // Log cleanup completion
  polycall_dop_audit_event_t audit_event = {
      .event_type = POLYCALL_DOP_AUDIT_COMPONENT_DESTROYED,
      .timestamp_ns = dop_lifecycle_get_timestamp_ns(),
      .component_id = component->component_id,
      .method_name = "cleanup",
      .error_code = POLYCALL_DOP_SUCCESS,
      .details = "Component cleanup completed successfully",
      .context_data = NULL,
      .context_size = 0};
  polycall_dop_audit_log(adapter_ctx, &audit_event);

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_lifecycle_setup_runtime_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_component_config_t *config) {
  if (!adapter_ctx || !component || !config) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Setup language-specific runtime context
  switch (config->language) {
  case POLYCALL_DOP_LANGUAGE_C:
    // C components don't need special runtime setup
    component->runtime_context = NULL;
    break;

  case POLYCALL_DOP_LANGUAGE_JAVASCRIPT:
    // JavaScript runtime context would be setup here
    // This would involve creating V8 isolate, context, etc.
    component->runtime_context = NULL; // Placeholder
    break;

  case POLYCALL_DOP_LANGUAGE_PYTHON:
    // Python runtime context would be setup here
    // This would involve initializing Python interpreter state
    component->runtime_context = NULL; // Placeholder
    break;

  case POLYCALL_DOP_LANGUAGE_JVM:
    // JVM runtime context would be setup here
    // This would involve creating JVM instance, class loaders, etc.
    component->runtime_context = NULL; // Placeholder
    break;

  default:
    return POLYCALL_DOP_ERROR_NOT_IMPLEMENTED;
  }

  // Copy language-specific configuration if provided
  if (config->language_specific_config && config->config_size > 0) {
    void *config_copy =
        polycall_memory_allocate(adapter_ctx->core_ctx, config->config_size);
    if (!config_copy) {
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }

    memcpy(config_copy, config->language_specific_config, config->config_size);
    // Store config_copy in component structure if needed
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_lifecycle_cleanup_runtime_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component) {
  if (!adapter_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Cleanup language-specific runtime context
  if (component->runtime_context) {
    switch (component->language) {
    case POLYCALL_DOP_LANGUAGE_C:
      // C components don't have runtime context
      break;

    case POLYCALL_DOP_LANGUAGE_JAVASCRIPT:
      // Cleanup JavaScript runtime (V8 isolate, etc.)
      // polycall_memory_free(adapter_ctx->core_ctx,
      // component->runtime_context);
      break;

    case POLYCALL_DOP_LANGUAGE_PYTHON:
      // Cleanup Python runtime
      // polycall_memory_free(adapter_ctx->core_ctx,
      // component->runtime_context);
      break;

    case POLYCALL_DOP_LANGUAGE_JVM:
      // Cleanup JVM runtime
      // polycall_memory_free(adapter_ctx->core_ctx,
      // component->runtime_context);
      break;

    default:
      break;
    }

    component->runtime_context = NULL;
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_lifecycle_validate_dependencies(
    polycall_dop_adapter_context_t *adapter_ctx,
    const polycall_dop_component_config_t *config) {
  if (!adapter_ctx || !config) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate language bridge is available
  polycall_dop_bridge_t *bridge = NULL;
  polycall_dop_error_t result =
      polycall_dop_bridge_get(adapter_ctx, config->language, &bridge);
  if (result != POLYCALL_DOP_SUCCESS || !bridge) {
    return POLYCALL_DOP_ERROR_BRIDGE_UNAVAILABLE;
  }

  // Additional dependency validation could be added here
  // For example, checking for required libraries, permissions, etc.

  return POLYCALL_DOP_SUCCESS;
}

static void dop_lifecycle_log_state_transition(
    polycall_dop_adapter_context_t *adapter_ctx, const char *component_id,
    polycall_dop_component_state_t from_state,
    polycall_dop_component_state_t to_state, polycall_dop_error_t result) {
  if (!adapter_ctx || !component_id) {
    return;
  }

  char details[256];
  snprintf(details, sizeof(details), "State transition: %s -> %s",
           polycall_dop_component_state_string(from_state),
           polycall_dop_component_state_string(to_state));

  polycall_dop_audit_event_t audit_event = {
      .event_type = POLYCALL_DOP_AUDIT_COMPONENT_CREATED, // Using this for
                                                          // lifecycle events
      .timestamp_ns = dop_lifecycle_get_timestamp_ns(),
      .component_id = component_id,
      .method_name = "state_transition",
      .error_code = result,
      .details = details,
      .context_data = NULL,
      .context_size = 0};

  polycall_dop_audit_log(adapter_ctx, &audit_event);
}

static uint64_t dop_lifecycle_get_timestamp_ns(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
  }
  return 0;
}

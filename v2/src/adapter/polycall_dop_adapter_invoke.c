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
 * @file polycall_dop_adapter_invoke.c
 * @brief DOP Adapter Component Method Invocation Implementation
 *
 * LibPolyCall DOP Adapter Framework - Method Invocation Engine
 * OBINexus Computing - Aegis Project Technical Infrastructure
 *
 * Implements secure component method invocation with comprehensive Zero Trust
 * validation, parameter marshalling, execution monitoring, and result handling.
 * Critical module for banking app security isolation (ads service vs bankcard).
 *
 * @version 1.0.0
 * @date 2025-06-09
 */

#include "polycall/core/dop/polycall_dop_adapter.h"
#include "polycall/core/polycall_core.h"
#include "polycall/core/polycall_memory.h"
#include "polycall/core/polycall_security.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <time.h>

/* ====================================================================
 * Internal Function Declarations
 * ==================================================================== */

static polycall_dop_error_t dop_invoke_validate_parameters(
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation,
    const polycall_dop_method_signature_t **method_signature);

static polycall_dop_error_t dop_invoke_security_check(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation,
    const polycall_dop_method_signature_t *method_signature);

static polycall_dop_error_t dop_invoke_prepare_execution_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation);

static polycall_dop_error_t dop_invoke_execute_with_monitoring(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation,
    const polycall_dop_method_signature_t *method_signature,
    polycall_dop_result_t *result);

static polycall_dop_error_t dop_invoke_validate_result(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_method_signature_t *method_signature,
    polycall_dop_result_t *result);

static polycall_dop_error_t dop_invoke_cleanup_execution_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component);

static polycall_dop_error_t
dop_value_validate_type(const polycall_dop_value_t *value,
                        polycall_dop_value_type_t expected_type);

static bool dop_invoke_timeout_check(uint64_t start_time_ns,
                                     uint32_t timeout_ms);

static uint64_t dop_get_monotonic_time_ns(void);

/* ====================================================================
 * Component Method Invocation - Primary API Implementation
 * ==================================================================== */

polycall_dop_error_t
polycall_dop_component_invoke(polycall_dop_adapter_context_t *adapter_ctx,
                              polycall_dop_component_t *component,
                              const polycall_dop_invocation_t *invocation,
                              polycall_dop_result_t *result) {
  polycall_dop_error_t error_code = POLYCALL_DOP_SUCCESS;
  const polycall_dop_method_signature_t *method_signature = NULL;
  uint64_t invocation_start_time = dop_get_monotonic_time_ns();

  // Zero Trust validation - verify all inputs
  if (!adapter_ctx || !component || !invocation || !result) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Initialize result structure
  memset(result, 0, sizeof(polycall_dop_result_t));
  result->error_code = POLYCALL_DOP_SUCCESS;
  result->execution_time_ms = 0;
  result->memory_used = 0;

  // Validate adapter context
  if (!adapter_ctx->is_initialized) {
    error_code = POLYCALL_DOP_ERROR_INVALID_STATE;
    goto cleanup;
  }

  // Validate component state - must be READY for invocation
  if (component->state != POLYCALL_DOP_COMPONENT_READY) {
    error_code = POLYCALL_DOP_ERROR_INVALID_STATE;
    goto cleanup;
  }

  // Transition component to EXECUTING state
  component->state = POLYCALL_DOP_COMPONENT_EXECUTING;

  // Step 1: Validate invocation parameters and find method signature
  error_code =
      dop_invoke_validate_parameters(component, invocation, &method_signature);
  if (error_code != POLYCALL_DOP_SUCCESS) {
    goto cleanup;
  }

  // Step 2: Perform comprehensive security validation
  error_code = dop_invoke_security_check(adapter_ctx, component, invocation,
                                         method_signature);
  if (error_code != POLYCALL_DOP_SUCCESS) {
    component->security_violations++;
    goto cleanup;
  }

  // Step 3: Prepare execution context with memory boundaries
  error_code =
      dop_invoke_prepare_execution_context(adapter_ctx, component, invocation);
  if (error_code != POLYCALL_DOP_SUCCESS) {
    goto cleanup;
  }

  // Step 4: Execute method with comprehensive monitoring
  error_code = dop_invoke_execute_with_monitoring(
      adapter_ctx, component, invocation, method_signature, result);
  if (error_code != POLYCALL_DOP_SUCCESS) {
    goto cleanup;
  }

  // Step 5: Validate execution result
  error_code = dop_invoke_validate_result(adapter_ctx, component,
                                          method_signature, result);
  if (error_code != POLYCALL_DOP_SUCCESS) {
    goto cleanup;
  }

  // Step 6: Update component statistics
  component->invocation_count++;
  uint64_t execution_time_ns =
      dop_get_monotonic_time_ns() - invocation_start_time;
  component->total_execution_time_ns += execution_time_ns;
  result->execution_time_ms = (uint32_t)(execution_time_ns / 1000000);

  // Log successful invocation
  polycall_dop_audit_event_t audit_event = {
      .event_type = POLYCALL_DOP_AUDIT_METHOD_INVOKED,
      .timestamp_ns = dop_get_monotonic_time_ns(),
      .component_id = component->component_id,
      .method_name = invocation->method_name,
      .error_code = POLYCALL_DOP_SUCCESS,
      .details = "Method invocation completed successfully",
      .context_data = NULL,
      .context_size = 0};
  polycall_dop_audit_log(adapter_ctx, &audit_event);

cleanup:
  // Step 7: Cleanup execution context
  dop_invoke_cleanup_execution_context(adapter_ctx, component);

  // Transition component back to appropriate state
  if (component->state == POLYCALL_DOP_COMPONENT_EXECUTING) {
    if (error_code == POLYCALL_DOP_SUCCESS) {
      component->state = POLYCALL_DOP_COMPONENT_READY;
    } else {
      component->state = POLYCALL_DOP_COMPONENT_ERROR;
    }
  }

  // Set result error code
  result->error_code = error_code;

  // Log error if invocation failed
  if (error_code != POLYCALL_DOP_SUCCESS) {
    polycall_dop_audit_event_t error_audit_event = {
        .event_type = POLYCALL_DOP_AUDIT_METHOD_INVOKED,
        .timestamp_ns = dop_get_monotonic_time_ns(),
        .component_id = component->component_id,
        .method_name = invocation->method_name,
        .error_code = error_code,
        .details = polycall_dop_error_string(error_code),
        .context_data = NULL,
        .context_size = 0};
    polycall_dop_audit_log(adapter_ctx, &error_audit_event);
  }

  return error_code;
}

polycall_dop_error_t
polycall_dop_invoke(polycall_dop_adapter_context_t *adapter_ctx,
                    const char *component_id, const char *method_name,
                    const polycall_dop_value_t *parameters,
                    size_t parameter_count, polycall_dop_result_t *result) {
  if (!adapter_ctx || !component_id || !method_name || !result) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (parameter_count > 0 && !parameters) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (parameter_count > POLYCALL_DOP_MAX_PARAMETERS) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Find component by ID
  polycall_dop_component_t *component = NULL;
  polycall_dop_error_t find_result =
      polycall_dop_component_find(adapter_ctx, component_id, &component);
  if (find_result != POLYCALL_DOP_SUCCESS) {
    return find_result;
  }

  // Create invocation structure
  polycall_dop_invocation_t invocation = {
      .method_name = method_name,
      .parameters = (polycall_dop_value_t *)
          parameters, // Cast away const for API compatibility
      .parameter_count = parameter_count,
      .execution_model = POLYCALL_DOP_EXEC_SYNCHRONOUS,
      .timeout_ms = POLYCALL_DOP_DEFAULT_TIMEOUT_MS,
      .user_context = NULL};

  // Invoke component method
  return polycall_dop_component_invoke(adapter_ctx, component, &invocation,
                                       result);
}

/* ====================================================================
 * Internal Helper Function Implementations
 * ==================================================================== */

static polycall_dop_error_t dop_invoke_validate_parameters(
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation,
    const polycall_dop_method_signature_t **method_signature) {
  if (!component || !invocation || !method_signature) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate method name
  if (!invocation->method_name || strlen(invocation->method_name) == 0) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (strlen(invocation->method_name) >= POLYCALL_DOP_MAX_METHOD_NAME_LENGTH) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Find method signature in component
  *method_signature = NULL;
  for (size_t i = 0; i < component->method_count; i++) {
    if (strcmp(component->methods[i].method_name, invocation->method_name) ==
        0) {
      *method_signature = &component->methods[i];
      break;
    }
  }

  if (!*method_signature) {
    return POLYCALL_DOP_ERROR_COMPONENT_NOT_FOUND; // Method not found
  }

  // Validate parameter count
  if (invocation->parameter_count != (*method_signature)->parameter_count) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate parameter types if parameters are provided
  if (invocation->parameter_count > 0) {
    if (!invocation->parameters) {
      return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
    }

    for (size_t i = 0; i < invocation->parameter_count; i++) {
      polycall_dop_error_t type_check = dop_value_validate_type(
          &invocation->parameters[i], (*method_signature)->parameter_types[i]);
      if (type_check != POLYCALL_DOP_SUCCESS) {
        return type_check;
      }
    }
  }

  // Validate execution model
  if (invocation->execution_model >= POLYCALL_DOP_EXEC_BATCH + 1) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate timeout
  if (invocation->timeout_ms == 0) {
    // Use method signature timeout or default
    uint32_t method_timeout = (*method_signature)->max_execution_time_ms;
    if (method_timeout == 0) {
      method_timeout = POLYCALL_DOP_DEFAULT_TIMEOUT_MS;
    }
    // Note: We can't modify the const invocation, so we'll use the timeout in
    // execution
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_invoke_security_check(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation,
    const polycall_dop_method_signature_t *method_signature) {
  if (!adapter_ctx || !component || !invocation || !method_signature) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Check if component has required permissions for this method
  polycall_dop_permission_flags_t required_permissions =
      method_signature->required_permissions;
  polycall_dop_permission_flags_t allowed_permissions =
      component->security_policy.allowed_permissions;
  polycall_dop_permission_flags_t denied_permissions =
      component->security_policy.denied_permissions;

  // Verify component has all required permissions
  if ((allowed_permissions & required_permissions) != required_permissions) {
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Verify no denied permissions are being used
  if ((denied_permissions & required_permissions) != 0) {
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Validate execution timeout against security policy
  uint32_t effective_timeout = invocation->timeout_ms;
  if (effective_timeout == 0) {
    effective_timeout = method_signature->max_execution_time_ms;
  }
  if (effective_timeout == 0) {
    effective_timeout = POLYCALL_DOP_DEFAULT_TIMEOUT_MS;
  }

  if (effective_timeout > component->security_policy.max_execution_time_ms) {
    return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
  }

  // Validate parameter data doesn't contain security violations
  for (size_t i = 0; i < invocation->parameter_count; i++) {
    const polycall_dop_value_t *param = &invocation->parameters[i];

    // Check string parameters for potential injection attacks
    if (param->type == POLYCALL_DOP_VALUE_STRING &&
        param->value.string_val.data) {
      const char *str_data = param->value.string_val.data;
      size_t str_length = param->value.string_val.length;

      // Basic sanity check - ensure string is null-terminated and length is
      // reasonable
      if (str_length > 0 && (strnlen(str_data, str_length + 1) != str_length)) {
        return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
      }

      // Check for suspicious patterns (basic example - can be extended)
      if (strstr(str_data, "../") != NULL || strstr(str_data, "..\\") != NULL) {
        return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
      }
    }

    // Check binary data parameters for size limits
    if (param->type == POLYCALL_DOP_VALUE_BYTES &&
        param->value.bytes_val.data) {
      // Ensure binary data doesn't exceed reasonable limits
      if (param->value.bytes_val.size > 1024 * 1024) { // 1MB limit
        return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
      }
    }
  }

  // Call component-specific security callback if provided
  if (component->security_callback) {
    polycall_dop_error_t callback_result = component->security_callback(
        component, invocation, component->user_data);
    if (callback_result != POLYCALL_DOP_SUCCESS) {
      return callback_result;
    }
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_invoke_prepare_execution_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation) {
  if (!adapter_ctx || !component || !invocation) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Prepare memory isolation boundaries if strict isolation is enabled
  if (component->security_policy.isolation_level >=
      POLYCALL_DOP_ISOLATION_STRICT) {
    // Memory boundary preparation would be implemented here
    // This is a placeholder for the memory isolation setup

    // Verify current memory usage doesn't exceed limits
    if (component->total_memory_allocated >
        component->security_policy.max_memory_usage) {
      return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
    }
  }

  // Initialize execution-specific security context
  if (adapter_ctx->zero_trust_enabled) {
    // Additional Zero Trust preparation would be implemented here
    // This could include setting up sandbox environments, etc.
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_invoke_execute_with_monitoring(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_invocation_t *invocation,
    const polycall_dop_method_signature_t *method_signature,
    polycall_dop_result_t *result) {
  if (!adapter_ctx || !component || !invocation || !method_signature ||
      !result) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  uint64_t execution_start_time = dop_get_monotonic_time_ns();
  polycall_dop_error_t execution_result = POLYCALL_DOP_SUCCESS;

  // Determine effective timeout
  uint32_t effective_timeout = invocation->timeout_ms;
  if (effective_timeout == 0) {
    effective_timeout = method_signature->max_execution_time_ms;
  }
  if (effective_timeout == 0) {
    effective_timeout = POLYCALL_DOP_DEFAULT_TIMEOUT_MS;
  }

  // Call component's invoke callback if available
  if (component->invoke_callback) {
    execution_result = component->invoke_callback(component, invocation, result,
                                                  component->user_data);
  } else {
    // If no callback is provided, this is an error
    execution_result = POLYCALL_DOP_ERROR_NOT_IMPLEMENTED;
  }

  // Check for timeout during execution
  uint64_t execution_time_ns =
      dop_get_monotonic_time_ns() - execution_start_time;
  uint32_t execution_time_ms = (uint32_t)(execution_time_ns / 1000000);

  if (execution_time_ms > effective_timeout) {
    return POLYCALL_DOP_ERROR_TIMEOUT;
  }

  // Update result timing information
  result->execution_time_ms = execution_time_ms;

  // Monitor memory usage during execution
  // This would integrate with the memory manager to track allocations
  // For now, we'll use a placeholder value
  result->memory_used =
      0; // Placeholder - actual implementation would track memory

  return execution_result;
}

static polycall_dop_error_t dop_invoke_validate_result(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component,
    const polycall_dop_method_signature_t *method_signature,
    polycall_dop_result_t *result) {
  if (!adapter_ctx || !component || !method_signature || !result) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate return value type matches method signature
  if (result->error_code == POLYCALL_DOP_SUCCESS) {
    polycall_dop_error_t type_check = dop_value_validate_type(
        &result->return_value, method_signature->return_type);
    if (type_check != POLYCALL_DOP_SUCCESS) {
      return type_check;
    }
  }

  // Validate execution time is within bounds
  if (result->execution_time_ms >
      component->security_policy.max_execution_time_ms) {
    return POLYCALL_DOP_ERROR_TIMEOUT;
  }

  // Validate memory usage is within bounds
  if (result->memory_used > component->security_policy.max_memory_usage) {
    return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_invoke_cleanup_execution_context(
    polycall_dop_adapter_context_t *adapter_ctx,
    polycall_dop_component_t *component) {
  if (!adapter_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Cleanup any execution-specific resources
  // This would include cleaning up temporary memory allocations,
  // resetting security contexts, etc.

  // For now, this is a placeholder implementation
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_value_validate_type(const polycall_dop_value_t *value,
                        polycall_dop_value_type_t expected_type) {
  if (!value) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Allow NULL values for any type
  if (value->type == POLYCALL_DOP_VALUE_NULL) {
    return POLYCALL_DOP_SUCCESS;
  }

  // Check for exact type match
  if (value->type == expected_type) {
    return POLYCALL_DOP_SUCCESS;
  }

  // Add type coercion rules here if needed
  // For now, we require exact type matches

  return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
}

static bool dop_invoke_timeout_check(uint64_t start_time_ns,
                                     uint32_t timeout_ms) {
  uint64_t current_time_ns = dop_get_monotonic_time_ns();
  uint64_t elapsed_ns = current_time_ns - start_time_ns;
  uint64_t timeout_ns = (uint64_t)timeout_ms * 1000000;

  return elapsed_ns >= timeout_ns;
}

static uint64_t dop_get_monotonic_time_ns(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
  }
  return 0; // Fallback for systems without high-resolution timer
}

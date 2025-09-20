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
 * @file polycall_dop_adapter_utils.c
 * @brief DOP Adapter Utility Functions Implementation
 *
 * LibPolyCall DOP Adapter Framework - Utility and Helper Functions
 * OBINexus Computing - Aegis Project Technical Infrastructure
 *
 * Implements utility functions for configuration validation, debugging,
 * serialization, and other common operations used throughout the DOP Adapter.
 *
 * @version 1.0.0
 * @date 2025-06-09
 */

#include "polycall/core/dop/polycall_dop_adapter.h"
#include "polycall/core/polycall_core.h"
#include "polycall/core/polycall_memory.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

/* ====================================================================
 * Internal Function Declarations
 * ==================================================================== */

static bool dop_utils_is_valid_identifier(const char *identifier);
static bool dop_utils_is_valid_version_string(const char *version);
static polycall_dop_error_t dop_utils_validate_method_signature(
    const polycall_dop_method_signature_t *signature);
static size_t dop_utils_calculate_value_size(const polycall_dop_value_t *value);
static polycall_dop_error_t
dop_utils_copy_string_safe(char *dest, size_t dest_size, const char *src);

/* ====================================================================
 * Value System Utility Functions
 * ==================================================================== */

/**
 * @brief Create a null value
 */
polycall_dop_value_t polycall_dop_value_create_null(void) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_NULL, .value = {0}};
  return value;
}

/**
 * @brief Create a boolean value
 */
polycall_dop_value_t polycall_dop_value_create_bool(bool bool_value) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_BOOL,
                                .value.bool_val = bool_value};
  return value;
}

/**
 * @brief Create a 32-bit integer value
 */
polycall_dop_value_t polycall_dop_value_create_int32(int32_t int_value) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_INT32,
                                .value.int32_val = int_value};
  return value;
}

/**
 * @brief Create a 64-bit integer value
 */
polycall_dop_value_t polycall_dop_value_create_int64(int64_t int_value) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_INT64,
                                .value.int64_val = int_value};
  return value;
}

/**
 * @brief Create a 32-bit unsigned integer value
 */
polycall_dop_value_t polycall_dop_value_create_uint32(uint32_t uint_value) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_UINT32,
                                .value.uint32_val = uint_value};
  return value;
}

/**
 * @brief Create a 64-bit unsigned integer value
 */
polycall_dop_value_t polycall_dop_value_create_uint64(uint64_t uint_value) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_UINT64,
                                .value.uint64_val = uint_value};
  return value;
}

/**
 * @brief Create a 32-bit float value
 */
polycall_dop_value_t polycall_dop_value_create_float32(float float_value) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_FLOAT32,
                                .value.float32_val = float_value};
  return value;
}

/**
 * @brief Create a 64-bit float value
 */
polycall_dop_value_t polycall_dop_value_create_float64(double double_value) {
  polycall_dop_value_t value = {.type = POLYCALL_DOP_VALUE_FLOAT64,
                                .value.float64_val = double_value};
  return value;
}

/**
 * @brief Create a string value (with memory allocation)
 */
polycall_dop_error_t
polycall_dop_value_create_string(polycall_core_context_t *core_ctx,
                                 const char *string_data,
                                 polycall_dop_value_t *value) {
  if (!core_ctx || !string_data || !value) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  size_t string_length = strlen(string_data);

  // Allocate memory for string copy
  char *string_copy =
      (char *)polycall_memory_allocate(core_ctx, string_length + 1);
  if (!string_copy) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  strcpy(string_copy, string_data);

  value->type = POLYCALL_DOP_VALUE_STRING;
  value->value.string_val.data = string_copy;
  value->value.string_val.length = string_length;

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Create a bytes value (with memory allocation)
 */
polycall_dop_error_t
polycall_dop_value_create_bytes(polycall_core_context_t *core_ctx,
                                const void *bytes_data, size_t bytes_size,
                                polycall_dop_value_t *value) {
  if (!core_ctx || !bytes_data || bytes_size == 0 || !value) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Allocate memory for bytes copy
  void *bytes_copy = polycall_memory_allocate(core_ctx, bytes_size);
  if (!bytes_copy) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  memcpy(bytes_copy, bytes_data, bytes_size);

  value->type = POLYCALL_DOP_VALUE_BYTES;
  value->value.bytes_val.data = bytes_copy;
  value->value.bytes_val.size = bytes_size;

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Free memory associated with a value
 */
polycall_dop_error_t
polycall_dop_value_destroy(polycall_core_context_t *core_ctx,
                           polycall_dop_value_t *value) {
  if (!core_ctx || !value) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  switch (value->type) {
  case POLYCALL_DOP_VALUE_STRING:
    if (value->value.string_val.data) {
      polycall_memory_free(core_ctx, value->value.string_val.data);
      value->value.string_val.data = NULL;
      value->value.string_val.length = 0;
    }
    break;

  case POLYCALL_DOP_VALUE_BYTES:
    if (value->value.bytes_val.data) {
      polycall_memory_free(core_ctx, value->value.bytes_val.data);
      value->value.bytes_val.data = NULL;
      value->value.bytes_val.size = 0;
    }
    break;

  case POLYCALL_DOP_VALUE_ARRAY:
    if (value->value.array_val.elements) {
      // Recursively destroy array elements
      for (size_t i = 0; i < value->value.array_val.count; i++) {
        polycall_dop_value_destroy(core_ctx,
                                   &value->value.array_val.elements[i]);
      }
      polycall_memory_free(core_ctx, value->value.array_val.elements);
      value->value.array_val.elements = NULL;
      value->value.array_val.count = 0;
    }
    break;

  default:
    // Primitive types don't need cleanup
    break;
  }

  // Reset to null value
  value->type = POLYCALL_DOP_VALUE_NULL;
  memset(&value->value, 0, sizeof(value->value));

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Copy a value (with deep copy for complex types)
 */
polycall_dop_error_t polycall_dop_value_copy(polycall_core_context_t *core_ctx,
                                             const polycall_dop_value_t *source,
                                             polycall_dop_value_t *dest) {
  if (!core_ctx || !source || !dest) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  dest->type = source->type;

  switch (source->type) {
  case POLYCALL_DOP_VALUE_NULL:
  case POLYCALL_DOP_VALUE_BOOL:
  case POLYCALL_DOP_VALUE_INT32:
  case POLYCALL_DOP_VALUE_INT64:
  case POLYCALL_DOP_VALUE_UINT32:
  case POLYCALL_DOP_VALUE_UINT64:
  case POLYCALL_DOP_VALUE_FLOAT32:
  case POLYCALL_DOP_VALUE_FLOAT64:
    // Simple copy for primitive types
    dest->value = source->value;
    break;

  case POLYCALL_DOP_VALUE_STRING:
    return polycall_dop_value_create_string(
        core_ctx, source->value.string_val.data, dest);

  case POLYCALL_DOP_VALUE_BYTES:
    return polycall_dop_value_create_bytes(core_ctx,
                                           source->value.bytes_val.data,
                                           source->value.bytes_val.size, dest);

  case POLYCALL_DOP_VALUE_ARRAY:
    // Deep copy array
    if (source->value.array_val.count > 0) {
      dest->value.array_val.elements =
          (polycall_dop_value_t *)polycall_memory_allocate(
              core_ctx,
              sizeof(polycall_dop_value_t) * source->value.array_val.count);
      if (!dest->value.array_val.elements) {
        return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
      }

      dest->value.array_val.count = source->value.array_val.count;

      for (size_t i = 0; i < source->value.array_val.count; i++) {
        polycall_dop_error_t result = polycall_dop_value_copy(
            core_ctx, &source->value.array_val.elements[i],
            &dest->value.array_val.elements[i]);
        if (result != POLYCALL_DOP_SUCCESS) {
          // Cleanup partial copy
          for (size_t j = 0; j < i; j++) {
            polycall_dop_value_destroy(core_ctx,
                                       &dest->value.array_val.elements[j]);
          }
          polycall_memory_free(core_ctx, dest->value.array_val.elements);
          return result;
        }
      }
    } else {
      dest->value.array_val.elements = NULL;
      dest->value.array_val.count = 0;
    }
    break;

  case POLYCALL_DOP_VALUE_COMPONENT_REF:
    // Component references are copied by value (shallow copy)
    dest->value.component_ref_val = source->value.component_ref_val;
    break;

  default:
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Configuration Validation Utilities
 * ==================================================================== */

/**
 * @brief Validate component configuration (extended validation)
 */
polycall_dop_error_t polycall_dop_component_config_validate_extended(
    const polycall_dop_component_config_t *config) {
  // First run basic validation
  polycall_dop_error_t basic_result =
      polycall_dop_component_config_validate(config);
  if (basic_result != POLYCALL_DOP_SUCCESS) {
    return basic_result;
  }

  // Extended validation checks

  // Validate component ID is a valid identifier
  if (!dop_utils_is_valid_identifier(config->component_id)) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate version string format
  if (!dop_utils_is_valid_version_string(config->version)) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate security policy consistency
  if (config->security_policy.allowed_permissions &
      config->security_policy.denied_permissions) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID; // Conflicting permissions
  }

  // Validate memory limits are reasonable
  if (config->security_policy.max_memory_usage == 0) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate execution timeout is reasonable
  if (config->security_policy.max_execution_time_ms == 0 ||
      config->security_policy.max_execution_time_ms > 3600000) { // Max 1 hour
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate method signatures
  for (size_t i = 0; i < config->method_count; i++) {
    polycall_dop_error_t method_result =
        dop_utils_validate_method_signature(&config->methods[i]);
    if (method_result != POLYCALL_DOP_SUCCESS) {
      return method_result;
    }
  }

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Create a default component configuration
 */
polycall_dop_error_t polycall_dop_component_config_create_default(
    const char *component_id, const char *component_name,
    polycall_dop_language_t language, polycall_dop_component_config_t *config) {
  if (!component_id || !component_name || !config) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Initialize config structure
  memset(config, 0, sizeof(polycall_dop_component_config_t));

  // Copy basic information with bounds checking
  polycall_dop_error_t result = dop_utils_copy_string_safe(
      (char *)config->component_id, POLYCALL_DOP_MAX_COMPONENT_ID_LENGTH,
      component_id);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  result = dop_utils_copy_string_safe((char *)config->component_name,
                                      POLYCALL_DOP_MAX_COMPONENT_NAME_LENGTH,
                                      component_name);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  result = dop_utils_copy_string_safe((char *)config->version,
                                      POLYCALL_DOP_MAX_VERSION_LENGTH, "1.0.0");
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  config->language = language;

  // Set default security policy based on language
  polycall_dop_isolation_level_t default_isolation;
  switch (language) {
  case POLYCALL_DOP_LANGUAGE_C:
    default_isolation = POLYCALL_DOP_ISOLATION_BASIC;
    break;
  case POLYCALL_DOP_LANGUAGE_JAVASCRIPT:
  case POLYCALL_DOP_LANGUAGE_PYTHON:
    default_isolation = POLYCALL_DOP_ISOLATION_STANDARD;
    break;
  case POLYCALL_DOP_LANGUAGE_JVM:
    default_isolation = POLYCALL_DOP_ISOLATION_STRICT;
    break;
  default:
    default_isolation = POLYCALL_DOP_ISOLATION_STANDARD;
    break;
  }

  result = polycall_dop_security_policy_create_default(
      default_isolation, &config->security_policy);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  config->memory_strategy = POLYCALL_DOP_MEMORY_POOL;
  config->methods = NULL;
  config->method_count = 0;
  config->language_specific_config = NULL;
  config->config_size = 0;

  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Debugging and Diagnostics Utilities
 * ==================================================================== */

/**
 * @brief Print component information for debugging
 */
polycall_dop_error_t
polycall_dop_component_debug_print(const polycall_dop_component_t *component,
                                   char *buffer, size_t buffer_size) {
  if (!component || !buffer || buffer_size == 0) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  int written = snprintf(
      buffer, buffer_size,
      "Component Debug Info:\n"
      "  ID: %s\n"
      "  Name: %s\n"
      "  Version: %s\n"
      "  State: %s\n"
      "  Language: %s\n"
      "  Isolation Level: %d\n"
      "  Invocation Count: %lu\n"
      "  Total Execution Time: %lu ns\n"
      "  Memory Allocated: %lu bytes\n"
      "  Security Violations: %lu\n"
      "  Method Count: %zu\n"
      "  Memory Region Count: %zu\n",
      component->component_id, component->component_name, component->version,
      polycall_dop_component_state_string(component->state),
      polycall_dop_language_string(component->language),
      (int)component->security_policy.isolation_level,
      component->invocation_count, component->total_execution_time_ns,
      component->total_memory_allocated, component->security_violations,
      component->method_count, component->region_count);

  if (written < 0 || (size_t)written >= buffer_size) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER; // Buffer too small
  }

  return POLYCALL_DOP_SUCCESS;
}

/**
 * @brief Print adapter statistics for debugging
 */
polycall_dop_error_t polycall_dop_adapter_debug_print_stats(
    const polycall_dop_adapter_context_t *adapter_ctx, char *buffer,
    size_t buffer_size) {
  if (!adapter_ctx || !buffer || buffer_size == 0) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  int written =
      snprintf(buffer, buffer_size,
               "DOP Adapter Statistics:\n"
               "  Initialized: %s\n"
               "  Zero Trust Enabled: %s\n"
               "  Component Count: %zu\n"
               "  Component Capacity: %zu\n"
               "  Bridge Count: %zu\n"
               "  Audit Buffer Count: %zu\n"
               "  Audit Buffer Size: %zu\n",
               adapter_ctx->is_initialized ? "Yes" : "No",
               adapter_ctx->zero_trust_enabled ? "Yes" : "No",
               adapter_ctx->component_count, adapter_ctx->component_capacity,
               adapter_ctx->bridge_count, adapter_ctx->audit_buffer_count,
               adapter_ctx->audit_buffer_size);

  if (written < 0 || (size_t)written >= buffer_size) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER; // Buffer too small
  }

  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Internal Helper Function Implementations
 * ==================================================================== */

static bool dop_utils_is_valid_identifier(const char *identifier) {
  if (!identifier || strlen(identifier) == 0) {
    return false;
  }

  // First character must be letter or underscore
  if (!isalpha(identifier[0]) && identifier[0] != '_') {
    return false;
  }

  // Remaining characters must be alphanumeric or underscore
  for (size_t i = 1; identifier[i] != '\0'; i++) {
    if (!isalnum(identifier[i]) && identifier[i] != '_') {
      return false;
    }
  }

  return true;
}

static bool dop_utils_is_valid_version_string(const char *version) {
  if (!version || strlen(version) == 0) {
    return false;
  }

  // Simple version validation: should match pattern like "1.2.3" or
  // "1.0.0-beta"
  bool has_digit = false;
  bool has_dot = false;

  for (size_t i = 0; version[i] != '\0'; i++) {
    char c = version[i];
    if (isdigit(c)) {
      has_digit = true;
    } else if (c == '.') {
      has_dot = true;
    } else if (c == '-' || c == '_' || isalpha(c)) {
      // Allow hyphens, underscores, and letters for pre-release versions
      continue;
    } else {
      return false; // Invalid character
    }
  }

  return has_digit && has_dot;
}

static polycall_dop_error_t dop_utils_validate_method_signature(
    const polycall_dop_method_signature_t *signature) {
  if (!signature) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate method name
  if (!signature->method_name ||
      !dop_utils_is_valid_identifier(signature->method_name)) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate parameter count
  if (signature->parameter_count > POLYCALL_DOP_MAX_PARAMETERS) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate parameter types array
  if (signature->parameter_count > 0 && !signature->parameter_types) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate parameter types
  for (size_t i = 0; i < signature->parameter_count; i++) {
    if (signature->parameter_types[i] >= POLYCALL_DOP_VALUE_COMPONENT_REF + 1) {
      return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
    }
  }

  // Validate return type
  if (signature->return_type >= POLYCALL_DOP_VALUE_COMPONENT_REF + 1) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate execution timeout
  if (signature->max_execution_time_ms > 3600000) { // Max 1 hour
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  return POLYCALL_DOP_SUCCESS;
}

static size_t
dop_utils_calculate_value_size(const polycall_dop_value_t *value) {
  if (!value) {
    return 0;
  }

  switch (value->type) {
  case POLYCALL_DOP_VALUE_NULL:
    return 0;
  case POLYCALL_DOP_VALUE_BOOL:
    return sizeof(bool);
  case POLYCALL_DOP_VALUE_INT32:
    return sizeof(int32_t);
  case POLYCALL_DOP_VALUE_INT64:
    return sizeof(int64_t);
  case POLYCALL_DOP_VALUE_UINT32:
    return sizeof(uint32_t);
  case POLYCALL_DOP_VALUE_UINT64:
    return sizeof(uint64_t);
  case POLYCALL_DOP_VALUE_FLOAT32:
    return sizeof(float);
  case POLYCALL_DOP_VALUE_FLOAT64:
    return sizeof(double);
  case POLYCALL_DOP_VALUE_STRING:
    return value->value.string_val.length + 1; // Include null terminator
  case POLYCALL_DOP_VALUE_BYTES:
    return value->value.bytes_val.size;
  case POLYCALL_DOP_VALUE_ARRAY: {
    size_t total_size = sizeof(size_t); // Array count
    for (size_t i = 0; i < value->value.array_val.count; i++) {
      total_size +=
          dop_utils_calculate_value_size(&value->value.array_val.elements[i]);
    }
    return total_size;
  }
  case POLYCALL_DOP_VALUE_COMPONENT_REF:
    return sizeof(void *); // Reference size
  default:
    return 0;
  }
}

static polycall_dop_error_t
dop_utils_copy_string_safe(char *dest, size_t dest_size, const char *src) {
  if (!dest || dest_size == 0 || !src) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  size_t src_length = strlen(src);
  if (src_length >= dest_size) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID; // String too long
  }

  strcpy(dest, src);
  return POLYCALL_DOP_SUCCESS;
}

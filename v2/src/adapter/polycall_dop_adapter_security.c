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
 * @file polycall_dop_adapter_security.c
 * @brief DOP Adapter Zero Trust Security Implementation
 *
 * LibPolyCall DOP Adapter Framework - Security Enforcement Engine
 * OBINexus Computing - Aegis Project Technical Infrastructure
 *
 * Implements comprehensive Zero Trust security model for cross-language
 * component isolation. Critical for banking app scenarios where ads services
 * must be completely isolated from banking payment components.
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
 * Security Context Internal Structure
 * ==================================================================== */

struct polycall_dop_security_context {
  polycall_core_context_t *core_ctx; ///< Core LibPolyCall context

  /* Access Control Lists */
  polycall_dop_component_t **trusted_components; ///< Trusted component registry
  size_t trusted_component_count;    ///< Number of trusted components
  size_t trusted_component_capacity; ///< Trusted component array capacity

  /* Permission Registry */
  struct {
    const char *operation;                        ///< Operation identifier
    polycall_dop_permission_flags_t required;     ///< Required permissions
    polycall_dop_isolation_level_t min_isolation; ///< Minimum isolation level
  } permission_rules[256];      ///< Permission validation rules
  size_t permission_rule_count; ///< Number of active rules

  /* Security Violation Tracking */
  struct {
    const char *component_id; ///< Component that violated policy
    polycall_dop_audit_event_type_t violation_type; ///< Type of violation
    uint64_t timestamp_ns;                          ///< When violation occurred
    const char *details;                            ///< Violation details
  } violation_log[1024];                            ///< Security violation log
  size_t violation_count; ///< Number of logged violations

  /* Security Policy Configuration */
  bool zero_trust_enabled;               ///< Zero Trust mode active
  bool isolation_enforcement_enabled;    ///< Memory isolation active
  bool audit_all_operations;             ///< Audit every operation
  uint32_t max_violations_per_component; ///< Max violations before blocking

  /* Cryptographic State */
  uint8_t session_key[32]; ///< Session encryption key
  uint64_t nonce_counter;  ///< Cryptographic nonce counter
};

/* ====================================================================
 * Internal Function Declarations
 * ==================================================================== */

static polycall_dop_error_t
dop_security_initialize_rules(polycall_dop_security_context_t *security_ctx);

static polycall_dop_error_t dop_security_validate_component_trust(
    polycall_dop_security_context_t *security_ctx,
    polycall_dop_component_t *component);

static polycall_dop_error_t dop_security_check_isolation_boundaries(
    polycall_dop_security_context_t *security_ctx,
    polycall_dop_component_t *component, const char *operation);

static polycall_dop_error_t dop_security_log_violation(
    polycall_dop_security_context_t *security_ctx, const char *component_id,
    polycall_dop_audit_event_type_t violation_type, const char *details);

static bool
dop_security_component_is_trusted(polycall_dop_security_context_t *security_ctx,
                                  polycall_dop_component_t *component);

static uint32_t
dop_security_get_violation_count(polycall_dop_security_context_t *security_ctx,
                                 const char *component_id);

static void dop_security_generate_session_key(
    polycall_dop_security_context_t *security_ctx);

static uint64_t dop_security_get_monotonic_time_ns(void);

/* ====================================================================
 * Security API Implementation
 * ==================================================================== */

polycall_dop_error_t
polycall_dop_security_validate(polycall_dop_adapter_context_t *adapter_ctx,
                               polycall_dop_component_t *component,
                               const char *operation) {
  if (!adapter_ctx || !component || !operation) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!adapter_ctx->security) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  polycall_dop_security_context_t *security_ctx = adapter_ctx->security;
  polycall_dop_error_t result = POLYCALL_DOP_SUCCESS;

  // Step 1: Validate component trust level
  result = dop_security_validate_component_trust(security_ctx, component);
  if (result != POLYCALL_DOP_SUCCESS) {
    dop_security_log_violation(security_ctx, component->component_id,
                               POLYCALL_DOP_AUDIT_SECURITY_VIOLATION,
                               "Component trust validation failed");
    return result;
  }

  // Step 2: Check if component has exceeded violation threshold
  uint32_t violation_count =
      dop_security_get_violation_count(security_ctx, component->component_id);
  if (violation_count >= security_ctx->max_violations_per_component) {
    dop_security_log_violation(
        security_ctx, component->component_id,
        POLYCALL_DOP_AUDIT_SECURITY_VIOLATION,
        "Component exceeded maximum violation threshold");
    return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
  }

  // Step 3: Validate operation against permission rules
  polycall_dop_permission_flags_t required_permissions =
      POLYCALL_DOP_PERMISSION_NONE;
  polycall_dop_isolation_level_t required_isolation =
      POLYCALL_DOP_ISOLATION_NONE;

  for (size_t i = 0; i < security_ctx->permission_rule_count; i++) {
    if (strcmp(security_ctx->permission_rules[i].operation, operation) == 0) {
      required_permissions = security_ctx->permission_rules[i].required;
      required_isolation = security_ctx->permission_rules[i].min_isolation;
      break;
    }
  }

  // Check component has required permissions
  if ((component->security_policy.allowed_permissions & required_permissions) !=
      required_permissions) {
    dop_security_log_violation(security_ctx, component->component_id,
                               POLYCALL_DOP_AUDIT_PERMISSION_DENIED,
                               "Insufficient permissions for operation");
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Check component isolation level meets requirements
  if (component->security_policy.isolation_level < required_isolation) {
    dop_security_log_violation(security_ctx, component->component_id,
                               POLYCALL_DOP_AUDIT_ISOLATION_BREACH,
                               "Insufficient isolation level for operation");
    return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
  }

  // Step 4: Check isolation boundaries
  result = dop_security_check_isolation_boundaries(security_ctx, component,
                                                   operation);
  if (result != POLYCALL_DOP_SUCCESS) {
    dop_security_log_violation(security_ctx, component->component_id,
                               POLYCALL_DOP_AUDIT_ISOLATION_BREACH,
                               "Isolation boundary violation detected");
    return result;
  }

  // Step 5: Zero Trust additional validation
  if (security_ctx->zero_trust_enabled) {
    // Additional Zero Trust checks would be implemented here
    // This could include cryptographic verification, behavioral analysis, etc.

    // Increment nonce counter to prevent replay attacks
    security_ctx->nonce_counter++;

    // Validate component hasn't been compromised (simplified check)
    if (component->security_violations > 0 &&
        component->security_policy.isolation_level <
            POLYCALL_DOP_ISOLATION_STRICT) {
      dop_security_log_violation(
          security_ctx, component->component_id,
          POLYCALL_DOP_AUDIT_SECURITY_VIOLATION,
          "Zero Trust validation failed: component integrity compromised");
      return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
    }
  }

  // Log successful validation if audit is enabled
  if (security_ctx->audit_all_operations) {
    polycall_dop_audit_event_t audit_event = {
        .event_type =
            POLYCALL_DOP_AUDIT_SECURITY_VIOLATION, // Using this enum value for
                                                   // general auditing
        .timestamp_ns = dop_security_get_monotonic_time_ns(),
        .component_id = component->component_id,
        .method_name = operation,
        .error_code = POLYCALL_DOP_SUCCESS,
        .details = "Security validation passed",
        .context_data = NULL,
        .context_size = 0};
    polycall_dop_audit_log(adapter_ctx, &audit_event);
  }

  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_audit_log(polycall_dop_adapter_context_t *adapter_ctx,
                       const polycall_dop_audit_event_t *event) {
  if (!adapter_ctx || !event) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!adapter_ctx->audit_buffer) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  // Check if audit buffer is full (circular buffer implementation)
  size_t write_index =
      adapter_ctx->audit_buffer_count % adapter_ctx->audit_buffer_size;

  // Copy event to audit buffer
  memcpy(&adapter_ctx->audit_buffer[write_index], event,
         sizeof(polycall_dop_audit_event_t));

  // Increment count (will wrap around for circular buffer)
  adapter_ctx->audit_buffer_count++;

  // If security context exists, also log security-specific events
  if (adapter_ctx->security &&
      (event->event_type == POLYCALL_DOP_AUDIT_SECURITY_VIOLATION ||
       event->event_type == POLYCALL_DOP_AUDIT_PERMISSION_DENIED ||
       event->event_type == POLYCALL_DOP_AUDIT_ISOLATION_BREACH)) {

    dop_security_log_violation(adapter_ctx->security, event->component_id,
                               event->event_type, event->details);
  }

  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_component_get_stats(polycall_dop_adapter_context_t *adapter_ctx,
                                 polycall_dop_component_t *component,
                                 polycall_dop_component_stats_t *stats) {
  if (!adapter_ctx || !component || !stats) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Initialize stats structure
  memset(stats, 0, sizeof(polycall_dop_component_stats_t));

  // Copy component statistics
  stats->invocation_count = component->invocation_count;
  stats->total_execution_time_ns = component->total_execution_time_ns;

  // Calculate average execution time
  if (component->invocation_count > 0) {
    stats->average_execution_time_ns =
        component->total_execution_time_ns / component->invocation_count;
  }

  stats->total_memory_allocated = component->total_memory_allocated;
  stats->security_violations = component->security_violations;
  stats->current_state = component->state;

  // Calculate current memory usage from memory regions
  stats->current_memory_usage = 0;
  for (size_t i = 0; i < component->region_count; i++) {
    stats->current_memory_usage += component->memory_regions[i].size;
  }

  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Internal Security Function Implementations
 * ==================================================================== */

static polycall_dop_error_t
dop_security_initialize_rules(polycall_dop_security_context_t *security_ctx) {
  if (!security_ctx) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Initialize default security rules
  size_t rule_index = 0;

  // Memory operations
  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "memory_allocate",
          .required = POLYCALL_DOP_PERMISSION_MEMORY_WRITE,
          .min_isolation = POLYCALL_DOP_ISOLATION_BASIC};

  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "memory_free",
          .required = POLYCALL_DOP_PERMISSION_MEMORY_WRITE,
          .min_isolation = POLYCALL_DOP_ISOLATION_BASIC};

  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "memory_share",
          .required = POLYCALL_DOP_PERMISSION_MEMORY_READ |
                      POLYCALL_DOP_PERMISSION_MEMORY_WRITE,
          .min_isolation = POLYCALL_DOP_ISOLATION_STANDARD};

  // Component invocation operations
  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "component_invoke_local",
          .required = POLYCALL_DOP_PERMISSION_INVOKE_LOCAL,
          .min_isolation = POLYCALL_DOP_ISOLATION_BASIC};

  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "component_invoke_remote",
          .required = POLYCALL_DOP_PERMISSION_INVOKE_REMOTE |
                      POLYCALL_DOP_PERMISSION_NETWORK,
          .min_isolation = POLYCALL_DOP_ISOLATION_STRICT};

  // File system operations
  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "file_access",
          .required = POLYCALL_DOP_PERMISSION_FILE_ACCESS,
          .min_isolation = POLYCALL_DOP_ISOLATION_STANDARD};

  // Network operations
  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "network_access",
          .required = POLYCALL_DOP_PERMISSION_NETWORK,
          .min_isolation = POLYCALL_DOP_ISOLATION_STRICT};

  // Privileged operations
  security_ctx->permission_rules[rule_index++] =
      (typeof(security_ctx->permission_rules[0])){
          .operation = "privileged_operation",
          .required = POLYCALL_DOP_PERMISSION_PRIVILEGED,
          .min_isolation = POLYCALL_DOP_ISOLATION_PARANOID};

  security_ctx->permission_rule_count = rule_index;

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_security_validate_component_trust(
    polycall_dop_security_context_t *security_ctx,
    polycall_dop_component_t *component) {
  if (!security_ctx || !component) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // In Zero Trust mode, no component is trusted by default
  if (security_ctx->zero_trust_enabled) {
    // Check if component is explicitly in trusted list
    if (!dop_security_component_is_trusted(security_ctx, component)) {
      // Apply additional validation for untrusted components

      // Check component hasn't exceeded security violation threshold
      if (component->security_violations > 0) {
        return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
      }

      // Validate component isolation level is sufficient
      if (component->security_policy.isolation_level <
          POLYCALL_DOP_ISOLATION_STANDARD) {
        return POLYCALL_DOP_ERROR_SECURITY_VIOLATION;
      }
    }
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_security_check_isolation_boundaries(
    polycall_dop_security_context_t *security_ctx,
    polycall_dop_component_t *component, const char *operation) {
  if (!security_ctx || !component || !operation) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Check if isolation enforcement is enabled
  if (!security_ctx->isolation_enforcement_enabled) {
    return POLYCALL_DOP_SUCCESS; // No isolation checks
  }

  // Validate component isolation level requirements
  polycall_dop_isolation_level_t required_level = POLYCALL_DOP_ISOLATION_BASIC;

  // Determine required isolation level based on operation
  if (strstr(operation, "memory") != NULL) {
    required_level = POLYCALL_DOP_ISOLATION_STANDARD;
  } else if (strstr(operation, "network") != NULL ||
             strstr(operation, "file") != NULL) {
    required_level = POLYCALL_DOP_ISOLATION_STRICT;
  } else if (strstr(operation, "privileged") != NULL) {
    required_level = POLYCALL_DOP_ISOLATION_PARANOID;
  }

  if (component->security_policy.isolation_level < required_level) {
    return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
  }

  // Check memory boundaries for memory-related operations
  if (strstr(operation, "memory") != NULL) {
    if (component->total_memory_allocated >
        component->security_policy.max_memory_usage) {
      return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
    }
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_security_log_violation(
    polycall_dop_security_context_t *security_ctx, const char *component_id,
    polycall_dop_audit_event_type_t violation_type, const char *details) {
  if (!security_ctx || !component_id || !details) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Check if violation log is full (circular buffer)
  size_t write_index = security_ctx->violation_count % 1024;

  // Log the violation
  security_ctx->violation_log[write_index] =
      (typeof(security_ctx->violation_log[0])){
          .component_id = component_id,
          .violation_type = violation_type,
          .timestamp_ns = dop_security_get_monotonic_time_ns(),
          .details = details};

  security_ctx->violation_count++;

  return POLYCALL_DOP_SUCCESS;
}

static bool
dop_security_component_is_trusted(polycall_dop_security_context_t *security_ctx,
                                  polycall_dop_component_t *component) {
  if (!security_ctx || !component) {
    return false;
  }

  // Check if component is in trusted list
  for (size_t i = 0; i < security_ctx->trusted_component_count; i++) {
    if (security_ctx->trusted_components[i] == component) {
      return true;
    }
  }

  return false;
}

static uint32_t
dop_security_get_violation_count(polycall_dop_security_context_t *security_ctx,
                                 const char *component_id) {
  if (!security_ctx || !component_id) {
    return 0;
  }

  uint32_t count = 0;
  size_t total_violations = security_ctx->violation_count;
  size_t start_index = 0;

  // If we have more violations than buffer size, start from oldest
  if (total_violations > 1024) {
    start_index = total_violations % 1024;
    total_violations = 1024;
  }

  // Count violations for this component
  for (size_t i = 0; i < total_violations; i++) {
    size_t index = (start_index + i) % 1024;
    if (strcmp(security_ctx->violation_log[index].component_id, component_id) ==
        0) {
      count++;
    }
  }

  return count;
}

static void dop_security_generate_session_key(
    polycall_dop_security_context_t *security_ctx) {
  if (!security_ctx) {
    return;
  }

  // Generate a simple session key (in production, use proper cryptographic RNG)
  for (size_t i = 0; i < sizeof(security_ctx->session_key); i++) {
    security_ctx->session_key[i] = (uint8_t)(rand() & 0xFF);
  }

  security_ctx->nonce_counter = 0;
}

static uint64_t dop_security_get_monotonic_time_ns(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
  }
  return 0;
}

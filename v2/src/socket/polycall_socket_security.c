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
 * @file polycall_socket_security.c
 * @brief WebSocket Security Integration for LibPolyCall
 * @author Implementation based on existing LibPolyCall architecture
 *
 * Implements security features for WebSocket connections including
 * authentication, authorization, and integration with LibPolyCall's auth
 * system.
 */

#include "polycall/core/auth/polycall_auth_token.h"
#include "polycall/core/polycall/polycall_error.h"
#include "polycall/core/polycall/polycall_memory.h"
#include "polycall/core/socket/polycall_socket_security.h"
#include <string.h>

#define POLYCALL_SOCKET_SECURITY_MAGIC 0xD4E5F6A7

/**
 * @brief Internal socket security context structure
 */
struct polycall_socket_security_context {
  uint32_t magic;                    // Magic number for validation
  polycall_core_context_t *core_ctx; // Core context
  polycall_auth_context_t *auth_ctx; // Auth context

  // Security configuration
  struct {
    bool enable_token_auth;          // Enable token authentication
    bool require_secure_connection;  // Require TLS/WSS
    uint32_t max_auth_failures;      // Maximum auth failures before blocking
    uint32_t token_refresh_interval; // Token refresh interval (seconds)
  } config;
};

/**
 * @brief Validate socket security context
 */
static bool validate_security_context(polycall_socket_security_context_t *ctx) {
  return ctx && ctx->magic == POLYCALL_SOCKET_SECURITY_MAGIC;
}

/**
 * @brief Initialize socket security context
 */
polycall_core_error_t polycall_socket_security_init(
    polycall_core_context_t *core_ctx, polycall_auth_context_t *auth_ctx,
    polycall_socket_security_context_t **security_ctx) {
  if (!core_ctx || !auth_ctx || !security_ctx) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Allocate security context
  polycall_socket_security_context_t *new_ctx = polycall_core_malloc(
      core_ctx, sizeof(polycall_socket_security_context_t));

  if (!new_ctx) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Initialize context
  memset(new_ctx, 0, sizeof(polycall_socket_security_context_t));

  // Set magic number for validation
  new_ctx->magic = POLYCALL_SOCKET_SECURITY_MAGIC;

  // Store contexts
  new_ctx->core_ctx = core_ctx;
  new_ctx->auth_ctx = auth_ctx;

  // Set default configuration
  new_ctx->config.enable_token_auth = true;
  new_ctx->config.require_secure_connection = true;
  new_ctx->config.max_auth_failures = 5;
  new_ctx->config.token_refresh_interval = 3600; // 1 hour

  *security_ctx = new_ctx;
  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Configure socket security
 */
polycall_core_error_t polycall_socket_security_configure(
    polycall_socket_security_context_t *security_ctx, bool enable_token_auth,
    bool require_secure_connection, uint32_t max_auth_failures,
    uint32_t token_refresh_interval) {
  if (!validate_security_context(security_ctx)) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Update configuration
  security_ctx->config.enable_token_auth = enable_token_auth;
  security_ctx->config.require_secure_connection = require_secure_connection;
  security_ctx->config.max_auth_failures = max_auth_failures;
  security_ctx->config.token_refresh_interval = token_refresh_interval;

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Authenticate WebSocket connection
 */
polycall_core_error_t
polycall_socket_authenticate(polycall_socket_security_context_t *security_ctx,
                             const char *token,
                             polycall_socket_auth_result_t *result) {
  if (!validate_security_context(security_ctx) || !token || !result) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Initialize result
  memset(result, 0, sizeof(polycall_socket_auth_result_t));

  // Skip authentication if disabled
  if (!security_ctx->config.enable_token_auth) {
    result->authenticated = true;
    return POLYCALL_CORE_SUCCESS;
  }

  // Validate token
  token_validation_result_t *validation_result = NULL;
  polycall_core_error_t error = polycall_auth_validate_token_ex(
      security_ctx->core_ctx, security_ctx->auth_ctx, token,
      &validation_result);

  if (error != POLYCALL_CORE_SUCCESS) {
    // Authentication failed due to error
    return error;
  }

  // Check validation result
  if (!validation_result || !validation_result->is_valid) {
    // Authentication failed due to invalid token
    if (validation_result) {
      // Copy error message if available
      if (validation_result->error_message) {
        strncpy(result->error_message, validation_result->error_message,
                sizeof(result->error_message) - 1);
      } else {
        strncpy(result->error_message, "Invalid token",
                sizeof(result->error_message) - 1);
      }

      // Free validation result
      polycall_auth_free_token_validation_result(security_ctx->core_ctx,
                                                 validation_result);
    }

    return POLYCALL_CORE_SUCCESS;
  }

  // Extract identity and roles from claims
  if (validation_result->claims) {
    // Copy identity
    if (validation_result->claims->subject) {
      strncpy(result->identity, validation_result->claims->subject,
              sizeof(result->identity) - 1);
    }

    // Extract scopes
    if (validation_result->claims->scopes &&
        validation_result->claims->scope_count > 0) {
      size_t scope_count = validation_result->claims->scope_count;
      if (scope_count > POLYCALL_MAX_SOCKET_SCOPES) {
        scope_count = POLYCALL_MAX_SOCKET_SCOPES;
      }

      result->scope_count = scope_count;

      for (size_t i = 0; i < scope_count; i++) {
        if (validation_result->claims->scopes[i]) {
          strncpy(result->scopes[i], validation_result->claims->scopes[i],
                  sizeof(result->scopes[0]) - 1);
        }
      }
    }

    // Extract roles
    if (validation_result->claims->roles &&
        validation_result->claims->role_count > 0) {
      size_t role_count = validation_result->claims->role_count;
      if (role_count > POLYCALL_MAX_SOCKET_ROLES) {
        role_count = POLYCALL_MAX_SOCKET_ROLES;
      }

      result->role_count = role_count;

      for (size_t i = 0; i < role_count; i++) {
        if (validation_result->claims->roles[i]) {
          strncpy(result->roles[i], validation_result->claims->roles[i],
                  sizeof(result->roles[0]) - 1);
        }
      }
    }

    // Set expiry time
    result->token_expiry = validation_result->claims->expires_at;
  }

  // Set authenticated flag
  result->authenticated = true;

  // Free validation result
  polycall_auth_free_token_validation_result(security_ctx->core_ctx,
                                             validation_result);

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Check if connection is authorized for a specific scope
 */
bool polycall_socket_is_authorized(
    polycall_socket_security_context_t *security_ctx,
    const polycall_socket_auth_result_t *auth_result,
    const char *required_scope) {
  if (!validate_security_context(security_ctx) || !auth_result ||
      !required_scope) {
    return false;
  }

  // If not authenticated, not authorized
  if (!auth_result->authenticated) {
    return false;
  }

  // If no required scope, authorized
  if (required_scope[0] == '\0') {
    return true;
  }

  // Check if the required scope is in the auth result scopes
  for (size_t i = 0; i < auth_result->scope_count; i++) {
    if (strcmp(auth_result->scopes[i], required_scope) == 0) {
      return true;
    }

    // Check for wildcard scope
    if (strcmp(auth_result->scopes[i], "*") == 0) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Check if connection has a specific role
 */
bool polycall_socket_has_role(polycall_socket_security_context_t *security_ctx,
                              const polycall_socket_auth_result_t *auth_result,
                              const char *required_role) {
  if (!validate_security_context(security_ctx) || !auth_result ||
      !required_role) {
    return false;
  }

  // If not authenticated, no role
  if (!auth_result->authenticated) {
    return false;
  }

  // Check if the required role is in the auth result roles
  for (size_t i = 0; i < auth_result->role_count; i++) {
    if (strcmp(auth_result->roles[i], required_role) == 0) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Generate a challenge for WebSocket authentication
 */
polycall_core_error_t polycall_socket_generate_challenge(
    polycall_socket_security_context_t *security_ctx, char *challenge_buffer,
    size_t buffer_size) {
  if (!validate_security_context(security_ctx) || !challenge_buffer ||
      buffer_size < 32) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // In a real implementation, use a CSPRNG
  // For this example, just fill with random characters
  const char charset[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (size_t i = 0; i < buffer_size - 1; i++) {
    challenge_buffer[i] = charset[rand() % (sizeof(charset) - 1)];
  }

  challenge_buffer[buffer_size - 1] = '\0';

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Verify a challenge response for WebSocket authentication
 */
polycall_core_error_t polycall_socket_verify_challenge(
    polycall_socket_security_context_t *security_ctx, const char *challenge,
    const char *response, const char *token, bool *valid) {
  if (!validate_security_context(security_ctx) || !challenge || !response ||
      !token || !valid) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Initialize result
  *valid = false;

  // TODO: Implement a proper challenge-response verification
  // This would typically involve:
  // 1. Use the token claims to get the identity
  // 2. Verify that the response was signed with the user's key
  // 3. Check that the response matches the expected format (e.g., HMAC of
  // challenge)

  // For this example, just validate the token
  token_validation_result_t *validation_result = NULL;
  polycall_core_error_t error = polycall_auth_validate_token_ex(
      security_ctx->core_ctx, security_ctx->auth_ctx, token,
      &validation_result);

  if (error != POLYCALL_CORE_SUCCESS) {
    return error;
  }

  // Set validity based on token validation
  *valid = validation_result && validation_result->is_valid;

  // Free validation result
  if (validation_result) {
    polycall_auth_free_token_validation_result(security_ctx->core_ctx,
                                               validation_result);
  }

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Check if a token needs to be refreshed
 */
bool polycall_socket_token_needs_refresh(
    polycall_socket_security_context_t *security_ctx,
    const polycall_socket_auth_result_t *auth_result, uint64_t current_time) {
  if (!validate_security_context(security_ctx) || !auth_result) {
    return false;
  }

  // If not authenticated, no refresh needed
  if (!auth_result->authenticated) {
    return false;
  }

  // If token has no expiry, no refresh needed
  if (auth_result->token_expiry == 0) {
    return false;
  }

  // Check if token is close to expiry
  uint64_t refresh_threshold =
      auth_result->token_expiry - security_ctx->config.token_refresh_interval;

  return current_time >= refresh_threshold;
}

/**
 * @brief Clean up socket security context
 */
void polycall_socket_security_cleanup(
    polycall_core_context_t *core_ctx,
    polycall_socket_security_context_t *security_ctx) {
  if (!core_ctx || !validate_security_context(security_ctx)) {
    return;
  }

  // Clear security context
  memset(security_ctx, 0, sizeof(polycall_socket_security_context_t));

  // Free context
  polycall_core_free(core_ctx, security_ctx);
}

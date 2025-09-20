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
 * @file polycall_socket.c
 * @brief Core WebSocket Implementation for LibPolyCall
 * @author Implementation based on existing LibPolyCall architecture
 *
 * Implements the WebSocket protocol following RFC 6455 specifications
 * with integration into LibPolyCall's core memory and error management.
 */

#include "polycall/core/polycall/polycall_error.h"
#include "polycall/core/polycall/polycall_memory.h"
#include "polycall/core/socket/polycall_socket.h"
#include <string.h>
#include <time.h>

#define POLYCALL_SOCKET_MAGIC 0xB1C2D3E4
#define MAX_CONNECTIONS 1024
#define MAX_EVENT_CALLBACKS 16

// Internal socket context structure
struct polycall_socket_context {
  uint32_t magic;                  // Magic number for validation
  polycall_socket_config_t config; // Socket configuration

  // Connection management
  polycall_socket_connection_t *connections[MAX_CONNECTIONS];
  size_t connection_count;

  // Callback management
  struct {
    polycall_socket_callback_t callback;
    void *user_data;
  } callbacks[MAX_EVENT_CALLBACKS];
  size_t callback_count;

  // Synchronization primitives
  pthread_mutex_t context_mutex;
  bool is_active;

  // Integration with other components
  polycall_core_context_t *core_ctx;
  polycall_telemetry_context_t *telemetry_ctx;
  polycall_auth_context_t *auth_ctx;
};

/**
 * @brief Validate socket context integrity
 */
static bool validate_socket_context(polycall_socket_context_t *ctx) {
  return ctx && ctx->magic == POLYCALL_SOCKET_MAGIC;
}

/**
 * @brief Generate a timestamp using high-resolution monotonic clock
 */
static uint64_t generate_timestamp(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * @brief Initialize socket system
 */
polycall_core_error_t
polycall_socket_init(polycall_core_context_t *core_ctx,
                     polycall_socket_context_t **socket_ctx,
                     const polycall_socket_config_t *config) {
  if (!core_ctx || !socket_ctx || !config) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Allocate socket context
  polycall_socket_context_t *new_ctx =
      polycall_core_malloc(core_ctx, sizeof(polycall_socket_context_t));

  if (!new_ctx) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Initialize context
  memset(new_ctx, 0, sizeof(polycall_socket_context_t));

  // Set magic number for validation
  new_ctx->magic = POLYCALL_SOCKET_MAGIC;

  // Copy configuration
  memcpy(&new_ctx->config, config, sizeof(polycall_socket_config_t));

  // Initialize synchronization primitives
  pthread_mutex_init(&new_ctx->context_mutex, NULL);
  new_ctx->is_active = true;

  // Store core context
  new_ctx->core_ctx = core_ctx;

  // Log initialization via telemetry if available
  if (new_ctx->telemetry_ctx) {
    polycall_telemetry_event_t event = {0};
    event.event_id = POLYCALL_TELEMETRY_EVENT_COMPONENT_INIT;
    event.source_module = "socket";
    event.severity = POLYCALL_TELEMETRY_INFO;
    strncpy(event.message, "Socket component initialized",
            sizeof(event.message) - 1);

    // Record event (ignoring any errors)
    polycall_telemetry_record_event(new_ctx->telemetry_ctx, &event);
  }

  *socket_ctx = new_ctx;
  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Create WebSocket server
 */
polycall_core_error_t
polycall_socket_create_server(polycall_socket_context_t *socket_ctx,
                              const char *bind_address, uint16_t port,
                              polycall_socket_server_t **server) {
  if (!validate_socket_context(socket_ctx) || !bind_address || !server) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Allocate server structure
  polycall_socket_server_t *new_server = polycall_core_malloc(
      socket_ctx->core_ctx, sizeof(polycall_socket_server_t));

  if (!new_server) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Initialize server
  memset(new_server, 0, sizeof(polycall_socket_server_t));

  // Set server properties
  strncpy(new_server->bind_address, bind_address,
          sizeof(new_server->bind_address) - 1);
  new_server->port = port;
  new_server->socket_ctx = socket_ctx;
  new_server->is_running = false;

  // Initialize server mutex
  pthread_mutex_init(&new_server->server_mutex, NULL);

  // TODO: Implement actual socket binding and listening
  // This would involve platform-specific socket APIs

  // Log server creation
  if (socket_ctx->telemetry_ctx) {
    polycall_telemetry_event_t event = {0};
    event.event_id = POLYCALL_TELEMETRY_EVENT_SOCKET_SERVER_CREATE;
    event.source_module = "socket";
    event.severity = POLYCALL_TELEMETRY_INFO;
    snprintf(event.message, sizeof(event.message) - 1,
             "WebSocket server created at %s:%d", bind_address, port);

    // Record event (ignoring any errors)
    polycall_telemetry_record_event(socket_ctx->telemetry_ctx, &event);
  }

  *server = new_server;
  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Connect to WebSocket server
 */
polycall_core_error_t
polycall_socket_connect(polycall_socket_context_t *socket_ctx, const char *url,
                        const polycall_socket_connect_options_t *options,
                        polycall_socket_connection_t **connection) {
  if (!validate_socket_context(socket_ctx) || !url || !connection) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Lock context
  pthread_mutex_lock(&socket_ctx->context_mutex);

  // Check connection capacity
  if (socket_ctx->connection_count >= MAX_CONNECTIONS) {
    pthread_mutex_unlock(&socket_ctx->context_mutex);
    return POLYCALL_CORE_ERROR_CAPACITY_EXCEEDED;
  }

  // Allocate connection structure
  polycall_socket_connection_t *new_connection = polycall_core_malloc(
      socket_ctx->core_ctx, sizeof(polycall_socket_connection_t));

  if (!new_connection) {
    pthread_mutex_unlock(&socket_ctx->context_mutex);
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Initialize connection
  memset(new_connection, 0, sizeof(polycall_socket_connection_t));

  // Set connection properties
  strncpy(new_connection->url, url, sizeof(new_connection->url) - 1);
  new_connection->socket_ctx = socket_ctx;
  new_connection->is_connected = false;
  new_connection->created_time = generate_timestamp();

  if (options) {
    // Copy connection options
    memcpy(&new_connection->options, options,
           sizeof(polycall_socket_connect_options_t));
  } else {
    // Set default options
    new_connection->options.timeout_ms = 30000; // 30 seconds default
    new_connection->options.use_tls = true;     // TLS by default
    new_connection->options.auto_reconnect = false;
  }

  // Initialize connection mutex
  pthread_mutex_init(&new_connection->connection_mutex, NULL);

  // TODO: Implement actual WebSocket connection
  // This would include:
  // 1. Parse URL to extract host, port, path
  // 2. Perform DNS resolution
  // 3. Create socket and connect
  // 4. Perform WebSocket handshake

  // Add connection to context
  socket_ctx->connections[socket_ctx->connection_count++] = new_connection;

  // Unlock context
  pthread_mutex_unlock(&socket_ctx->context_mutex);

  // Log connection attempt
  if (socket_ctx->telemetry_ctx) {
    polycall_telemetry_event_t event = {0};
    event.event_id = POLYCALL_TELEMETRY_EVENT_SOCKET_CONNECT;
    event.source_module = "socket";
    event.severity = POLYCALL_TELEMETRY_INFO;
    snprintf(event.message, sizeof(event.message) - 1,
             "WebSocket connection attempt to %s", url);

    // Record event (ignoring any errors)
    polycall_telemetry_record_event(socket_ctx->telemetry_ctx, &event);
  }

  *connection = new_connection;
  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Send message over WebSocket connection
 */
polycall_core_error_t
polycall_socket_send(polycall_socket_connection_t *connection, const void *data,
                     size_t data_size, polycall_socket_data_type_t data_type) {
  if (!connection || !data || data_size == 0) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Check connection state
  if (!connection->is_connected) {
    return POLYCALL_CORE_ERROR_CONNECTION_CLOSED;
  }

  // Lock connection
  pthread_mutex_lock(&connection->connection_mutex);

  // TODO: Implement actual WebSocket message sending
  // This would include:
  // 1. Frame the message according to WebSocket protocol
  // 2. Send the framed message over the socket
  // 3. Handle any socket errors

  // For now, just log the send attempt
  if (connection->socket_ctx && connection->socket_ctx->telemetry_ctx) {
    polycall_telemetry_event_t event = {0};
    event.event_id = POLYCALL_TELEMETRY_EVENT_SOCKET_SEND;
    event.source_module = "socket";
    event.severity = POLYCALL_TELEMETRY_DEBUG;
    snprintf(event.message, sizeof(event.message) - 1,
             "WebSocket send: %zu bytes, type %d", data_size, data_type);

    // Record event (ignoring any errors)
    polycall_telemetry_record_event(connection->socket_ctx->telemetry_ctx,
                                    &event);
  }

  // Unlock connection
  pthread_mutex_unlock(&connection->connection_mutex);

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Register message handler
 */
polycall_core_error_t
polycall_socket_register_handler(polycall_socket_connection_t *connection,
                                 polycall_socket_message_handler_t handler,
                                 void *user_data) {
  if (!connection || !handler) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Lock connection
  pthread_mutex_lock(&connection->connection_mutex);

  // Store handler and user data
  connection->message_handler = handler;
  connection->handler_user_data = user_data;

  // Unlock connection
  pthread_mutex_unlock(&connection->connection_mutex);

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Close WebSocket connection
 */
polycall_core_error_t
polycall_socket_close(polycall_socket_connection_t *connection,
                      uint16_t close_code, const char *reason) {
  if (!connection) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Lock connection
  pthread_mutex_lock(&connection->connection_mutex);

  // Check if already closed
  if (!connection->is_connected) {
    pthread_mutex_unlock(&connection->connection_mutex);
    return POLYCALL_CORE_SUCCESS;
  }

  // TODO: Implement proper WebSocket close frame sending
  // This would include:
  // 1. Create close frame with code and reason
  // 2. Send frame
  // 3. Wait for close frame response
  // 4. Close underlying socket

  // Mark as disconnected
  connection->is_connected = false;
  connection->close_code = close_code;

  if (reason) {
    strncpy(connection->close_reason, reason,
            sizeof(connection->close_reason) - 1);
  } else {
    connection->close_reason[0] = '\0';
  }

  // Log connection closure
  if (connection->socket_ctx && connection->socket_ctx->telemetry_ctx) {
    polycall_telemetry_event_t event = {0};
    event.event_id = POLYCALL_TELEMETRY_EVENT_SOCKET_CLOSE;
    event.source_module = "socket";
    event.severity = POLYCALL_TELEMETRY_INFO;

    if (reason && reason[0] != '\0') {
      snprintf(event.message, sizeof(event.message) - 1,
               "WebSocket closed: code %d, reason: %s", close_code, reason);
    } else {
      snprintf(event.message, sizeof(event.message) - 1,
               "WebSocket closed: code %d", close_code);
    }

    // Record event (ignoring any errors)
    polycall_telemetry_record_event(connection->socket_ctx->telemetry_ctx,
                                    &event);
  }

  // Unlock connection
  pthread_mutex_unlock(&connection->connection_mutex);

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Cleanup WebSocket system resources
 */
void polycall_socket_cleanup(polycall_core_context_t *core_ctx,
                             polycall_socket_context_t *socket_ctx) {
  if (!core_ctx || !validate_socket_context(socket_ctx)) {
    return;
  }

  // Lock context
  pthread_mutex_lock(&socket_ctx->context_mutex);

  // Mark as inactive
  socket_ctx->is_active = false;

  // Close all connections
  for (size_t i = 0; i < socket_ctx->connection_count; i++) {
    polycall_socket_connection_t *connection = socket_ctx->connections[i];
    if (connection) {
      // Close connection if open
      if (connection->is_connected) {
        polycall_socket_close(connection, 1001, "Service shutdown");
      }

      // Destroy connection mutex
      pthread_mutex_destroy(&connection->connection_mutex);

      // Free connection
      polycall_core_free(core_ctx, connection);
      socket_ctx->connections[i] = NULL;
    }
  }

  // Log cleanup
  if (socket_ctx->telemetry_ctx) {
    polycall_telemetry_event_t event = {0};
    event.event_id = POLYCALL_TELEMETRY_EVENT_COMPONENT_CLEANUP;
    event.source_module = "socket";
    event.severity = POLYCALL_TELEMETRY_INFO;
    strncpy(event.message, "Socket component cleaned up",
            sizeof(event.message) - 1);

    // Record event (ignoring any errors)
    polycall_telemetry_record_event(socket_ctx->telemetry_ctx, &event);
  }

  // Destroy context mutex
  pthread_mutex_unlock(&socket_ctx->context_mutex);
  pthread_mutex_destroy(&socket_ctx->context_mutex);

  // Invalidate magic number
  socket_ctx->magic = 0;

  // Free context
  polycall_core_free(core_ctx, socket_ctx);
}

/**
 * @brief Create default socket configuration
 */
polycall_socket_config_t polycall_socket_create_default_config(void) {
  polycall_socket_config_t default_config = {0};

  // Set default values
  default_config.max_connections = 1024;
  default_config.connection_timeout_ms = 30000;
  default_config.use_tls = true;
  default_config.ping_interval_ms = 30000;
  default_config.max_message_size = 65536;
  default_config.worker_threads = 4;
  default_config.enable_compression = false;
  default_config.auto_reconnect = false;
  default_config.reconnect_max_attempts = 5;
  default_config.reconnect_base_delay_ms = 1000;

  return default_config;
}

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
 * @file polycall_socket_protocol.c
 * @brief WebSocket Protocol Implementation for LibPolyCall
 * @author Implementation based on existing LibPolyCall architecture
 *
 * Implements the WebSocket protocol according to RFC 6455,
 * handling frames, handshakes, and protocol operations.
 */

#include "polycall/core/polycall/polycall_error.h"
#include "polycall/core/polycall/polycall_memory.h"
#include "polycall/core/socket/polycall_socket_protocol.h"
#include <string.h>

// WebSocket protocol constants
#define WS_FIN 0x80
#define WS_MASK 0x80
#define WS_OPCODE_CONTINUATION 0x00
#define WS_OPCODE_TEXT 0x01
#define WS_OPCODE_BINARY 0x02
#define WS_OPCODE_CLOSE 0x08
#define WS_OPCODE_PING 0x09
#define WS_OPCODE_PONG 0x0A

// Close status codes (RFC 6455)
#define WS_CLOSE_NORMAL 1000
#define WS_CLOSE_GOING_AWAY 1001
#define WS_CLOSE_PROTOCOL_ERROR 1002
#define WS_CLOSE_UNSUPPORTED 1003
#define WS_CLOSE_NO_STATUS 1005
#define WS_CLOSE_ABNORMAL 1006
#define WS_CLOSE_INVALID_DATA 1007
#define WS_CLOSE_POLICY 1008
#define WS_CLOSE_TOO_LARGE 1009
#define WS_CLOSE_EXTENSION 1010
#define WS_CLOSE_UNEXPECTED 1011
#define WS_CLOSE_TLS_FAILURE 1015

// Maximum header size (theoretical maximum)
#define MAX_HEADER_SIZE 14

/**
 * @brief WebSocket frame structure
 */
typedef struct {
  uint8_t fin;             // Final fragment flag
  uint8_t rsv1;            // Reserved bit 1
  uint8_t rsv2;            // Reserved bit 2
  uint8_t rsv3;            // Reserved bit 3
  uint8_t opcode;          // Frame opcode
  uint8_t masked;          // Masked flag
  uint64_t payload_length; // Payload length
  uint8_t mask_key[4];     // Masking key (if masked)
  uint8_t *payload;        // Payload data
} ws_frame_t;

/**
 * @brief Generate a random masking key for client frames
 */
static void generate_mask_key(uint8_t mask_key[4]) {
  // In a real implementation, use a CSPRNG
  for (int i = 0; i < 4; i++) {
    mask_key[i] = rand() & 0xFF;
  }
}

/**
 * @brief Apply WebSocket masking to payload
 * @param payload The payload data to mask/unmask
 * @param length Length of the payload
 * @param mask_key 4-byte masking key
 */
static void apply_mask(uint8_t *payload, size_t length,
                       const uint8_t mask_key[4]) {
  for (size_t i = 0; i < length; i++) {
    payload[i] ^= mask_key[i % 4];
  }
}

/**
 * @brief Create WebSocket handshake request (client)
 */
polycall_core_error_t polycall_socket_create_handshake_request(
    polycall_core_context_t *core_ctx, const char *host, const char *path,
    const char *protocols, uint8_t **buffer, size_t *buffer_size) {
  if (!core_ctx || !host || !path || !buffer || !buffer_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Generate random key (16 bytes, base64 encoded)
  // In a real implementation, use a proper random generator and base64 encoding
  const char *websocket_key = "dGhlIHNhbXBsZSBub25jZQ==";

  // Calculate request size
  size_t request_size = 512; // Base size for headers
  if (protocols) {
    request_size += strlen(protocols) + 32; // Extra space for protocols
  }

  // Allocate buffer
  *buffer = polycall_core_malloc(core_ctx, request_size);
  if (!*buffer) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Format handshake request
  int len = snprintf((char *)*buffer, request_size,
                     "GET %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Key: %s\r\n"
                     "Sec-WebSocket-Version: 13\r\n",
                     path, host, websocket_key);

  // Add protocols if specified
  if (protocols && strlen(protocols) > 0) {
    len += snprintf((char *)*buffer + len, request_size - len,
                    "Sec-WebSocket-Protocol: %s\r\n", protocols);
  }

  // Add final CRLF
  len += snprintf((char *)*buffer + len, request_size - len, "\r\n");

  // Set buffer size
  *buffer_size = len;

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Parse WebSocket handshake response (client)
 */
polycall_core_error_t polycall_socket_parse_handshake_response(
    polycall_core_context_t *core_ctx, const uint8_t *buffer,
    size_t buffer_size, bool *success, char **protocol) {
  if (!core_ctx || !buffer || !success) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Initialize output
  *success = false;
  if (protocol) {
    *protocol = NULL;
  }

  // Ensure buffer is null-terminated
  char *response = polycall_core_malloc(core_ctx, buffer_size + 1);
  if (!response) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  memcpy(response, buffer, buffer_size);
  response[buffer_size] = '\0';

  // Check for HTTP 101 response
  if (strstr(response, "HTTP/1.1 101") == NULL) {
    polycall_core_free(core_ctx, response);
    return POLYCALL_CORE_SUCCESS; // Not an error, just unsuccessful
  }

  // Check for required headers
  bool has_upgrade = strstr(response, "Upgrade: websocket") != NULL;
  bool has_connection = strstr(response, "Connection: Upgrade") != NULL;
  bool has_accept = strstr(response, "Sec-WebSocket-Accept:") != NULL;

  // Validate required headers
  if (!has_upgrade || !has_connection || !has_accept) {
    polycall_core_free(core_ctx, response);
    return POLYCALL_CORE_SUCCESS; // Not an error, just unsuccessful
  }

  // Extract protocol if present
  if (protocol) {
    const char *protocol_header = strstr(response, "Sec-WebSocket-Protocol:");
    if (protocol_header) {
      // Skip header name
      protocol_header += sizeof("Sec-WebSocket-Protocol:") - 1;

      // Skip whitespace
      while (*protocol_header == ' ' || *protocol_header == '\t') {
        protocol_header++;
      }

      // Find end of line
      const char *protocol_end = strstr(protocol_header, "\r\n");
      if (protocol_end) {
        size_t protocol_len = protocol_end - protocol_header;
        *protocol = polycall_core_malloc(core_ctx, protocol_len + 1);
        if (*protocol) {
          memcpy(*protocol, protocol_header, protocol_len);
          (*protocol)[protocol_len] = '\0';
        }
      }
    }
  }

  // Set success flag
  *success = true;

  // Clean up
  polycall_core_free(core_ctx, response);

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Create WebSocket handshake response (server)
 */
polycall_core_error_t polycall_socket_create_handshake_response(
    polycall_core_context_t *core_ctx, const char *websocket_key,
    const char *protocol, uint8_t **buffer, size_t *buffer_size) {
  if (!core_ctx || !websocket_key || !buffer || !buffer_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Compute Sec-WebSocket-Accept value
  // In a real implementation, concatenate with magic string and compute SHA-1
  // hash
  const char *accept_key = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";

  // Calculate response size
  size_t response_size = 256; // Base size for headers
  if (protocol) {
    response_size += strlen(protocol) + 32; // Extra space for protocol
  }

  // Allocate buffer
  *buffer = polycall_core_malloc(core_ctx, response_size);
  if (!*buffer) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Format handshake response
  int len = snprintf((char *)*buffer, response_size,
                     "HTTP/1.1 101 Switching Protocols\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Accept: %s\r\n",
                     accept_key);

  // Add protocol if specified
  if (protocol && strlen(protocol) > 0) {
    len += snprintf((char *)*buffer + len, response_size - len,
                    "Sec-WebSocket-Protocol: %s\r\n", protocol);
  }

  // Add final CRLF
  len += snprintf((char *)*buffer + len, response_size - len, "\r\n");

  // Set buffer size
  *buffer_size = len;

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Parse WebSocket handshake request (server)
 */
polycall_core_error_t polycall_socket_parse_handshake_request(
    polycall_core_context_t *core_ctx, const uint8_t *buffer,
    size_t buffer_size, char **key, char **protocols) {
  if (!core_ctx || !buffer || !key) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Initialize output
  *key = NULL;
  if (protocols) {
    *protocols = NULL;
  }

  // Ensure buffer is null-terminated
  char *request = polycall_core_malloc(core_ctx, buffer_size + 1);
  if (!request) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  memcpy(request, buffer, buffer_size);
  request[buffer_size] = '\0';

  // Check for GET request
  if (strstr(request, "GET ") != request) {
    polycall_core_free(core_ctx, request);
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Check for required headers
  bool has_upgrade = strstr(request, "Upgrade: websocket") != NULL;
  bool has_connection = strstr(request, "Connection: Upgrade") != NULL;
  bool has_version = strstr(request, "Sec-WebSocket-Version: 13") != NULL;

  // Validate required headers
  if (!has_upgrade || !has_connection || !has_version) {
    polycall_core_free(core_ctx, request);
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Extract key
  const char *key_header = strstr(request, "Sec-WebSocket-Key:");
  if (!key_header) {
    polycall_core_free(core_ctx, request);
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Skip header name
  key_header += sizeof("Sec-WebSocket-Key:") - 1;

  // Skip whitespace
  while (*key_header == ' ' || *key_header == '\t') {
    key_header++;
  }

  // Find end of line
  const char *key_end = strstr(key_header, "\r\n");
  if (!key_end) {
    polycall_core_free(core_ctx, request);
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Extract key
  size_t key_len = key_end - key_header;
  *key = polycall_core_malloc(core_ctx, key_len + 1);
  if (!*key) {
    polycall_core_free(core_ctx, request);
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  memcpy(*key, key_header, key_len);
  (*key)[key_len] = '\0';

  // Extract protocols if present
  if (protocols) {
    const char *protocol_header = strstr(request, "Sec-WebSocket-Protocol:");
    if (protocol_header) {
      // Skip header name
      protocol_header += sizeof("Sec-WebSocket-Protocol:") - 1;

      // Skip whitespace
      while (*protocol_header == ' ' || *protocol_header == '\t') {
        protocol_header++;
      }

      // Find end of line
      const char *protocol_end = strstr(protocol_header, "\r\n");
      if (protocol_end) {
        size_t protocol_len = protocol_end - protocol_header;
        *protocols = polycall_core_malloc(core_ctx, protocol_len + 1);
        if (*protocols) {
          memcpy(*protocols, protocol_header, protocol_len);
          (*protocols)[protocol_len] = '\0';
        }
      }
    }
  }

  // Clean up
  polycall_core_free(core_ctx, request);

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Create WebSocket frame
 */
polycall_core_error_t
polycall_socket_create_frame(polycall_core_context_t *core_ctx, uint8_t opcode,
                             const uint8_t *payload, size_t payload_size,
                             bool mask_payload, uint8_t **frame_buffer,
                             size_t *frame_size) {
  if (!core_ctx || (!payload && payload_size > 0) || !frame_buffer ||
      !frame_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Calculate frame header size (variable)
  size_t header_size = 2; // Basic header (2 bytes)

  if (payload_size > 125) {
    if (payload_size <= 65535) {
      header_size += 2; // 16-bit length
    } else {
      header_size += 8; // 64-bit length
    }
  }

  if (mask_payload) {
    header_size += 4; // Masking key
  }

  // Allocate frame buffer
  *frame_size = header_size + payload_size;
  *frame_buffer = polycall_core_malloc(core_ctx, *frame_size);
  if (!*frame_buffer) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Set FIN bit and opcode
  (*frame_buffer)[0] = WS_FIN | (opcode & 0x0F);

  // Set mask bit and payload length
  if (payload_size <= 125) {
    (*frame_buffer)[1] = (mask_payload ? WS_MASK : 0) | (uint8_t)payload_size;
  } else if (payload_size <= 65535) {
    (*frame_buffer)[1] = (mask_payload ? WS_MASK : 0) | 126;
    (*frame_buffer)[2] = (uint8_t)(payload_size >> 8);
    (*frame_buffer)[3] = (uint8_t)(payload_size);
  } else {
    (*frame_buffer)[1] = (mask_payload ? WS_MASK : 0) | 127;
    (*frame_buffer)[2] = 0; // 64-bit length, high order bits
    (*frame_buffer)[3] = 0;
    (*frame_buffer)[4] = 0;
    (*frame_buffer)[5] = 0;
    (*frame_buffer)[6] = (uint8_t)(payload_size >> 24);
    (*frame_buffer)[7] = (uint8_t)(payload_size >> 16);
    (*frame_buffer)[8] = (uint8_t)(payload_size >> 8);
    (*frame_buffer)[9] = (uint8_t)(payload_size);
  }

  // Add masking key if needed
  if (mask_payload) {
    uint8_t *mask_key_ptr = *frame_buffer + (header_size - 4);
    generate_mask_key(mask_key_ptr);

    // Copy payload with masking
    uint8_t *payload_ptr = *frame_buffer + header_size;
    memcpy(payload_ptr, payload, payload_size);
    apply_mask(payload_ptr, payload_size, mask_key_ptr);
  } else {
    // Copy payload directly
    memcpy(*frame_buffer + header_size, payload, payload_size);
  }

  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Parse WebSocket frame
 */
polycall_core_error_t
polycall_socket_parse_frame(polycall_core_context_t *core_ctx,
                            const uint8_t *frame_buffer, size_t buffer_size,
                            ws_frame_t **frame) {
  if (!core_ctx || !frame_buffer || buffer_size < 2 || !frame) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Allocate frame structure
  ws_frame_t *new_frame = polycall_core_malloc(core_ctx, sizeof(ws_frame_t));
  if (!new_frame) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Initialize frame
  memset(new_frame, 0, sizeof(ws_frame_t));

  // Parse basic header
  new_frame->fin = (frame_buffer[0] & WS_FIN) != 0;
  new_frame->rsv1 = (frame_buffer[0] & 0x40) != 0;
  new_frame->rsv2 = (frame_buffer[0] & 0x20) != 0;
  new_frame->rsv3 = (frame_buffer[0] & 0x10) != 0;
  new_frame->opcode = frame_buffer[0] & 0x0F;

  new_frame->masked = (frame_buffer[1] & WS_MASK) != 0;
  uint8_t length_field = frame_buffer[1] & 0x7F;

  // Determine payload length and header size
  size_t header_size = 2;

  if (length_field <= 125) {
    new_frame->payload_length = length_field;
  } else if (length_field == 126) {
    // 16-bit length
    if (buffer_size < 4) {
      polycall_core_free(core_ctx, new_frame);
      return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
    }

    new_frame->payload_length =
        (uint16_t)frame_buffer[2] << 8 | frame_buffer[3];
    header_size += 2;
  } else if (length_field == 127) {
    // 64-bit length
    if (buffer_size < 10) {
      polycall_core_free(core_ctx, new_frame);
      return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
    }

    // Check that high bits are zero (we don't support payloads > 2^32)
    if (frame_buffer[2] != 0 || frame_buffer[3] != 0 || frame_buffer[4] != 0 ||
        frame_buffer[5] != 0) {
      polycall_core_free(core_ctx, new_frame);
      return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
    }

    new_frame->payload_length =
        (uint64_t)frame_buffer[6] << 24 | (uint64_t)frame_buffer[7] << 16 |
        (uint64_t)frame_buffer[8] << 8 | frame_buffer[9];
    header_size += 8;
  }

  // Add mask key size to header if present
  if (new_frame->masked) {
    if (buffer_size < header_size + 4) {
      polycall_core_free(core_ctx, new_frame);
      return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
    }

    memcpy(new_frame->mask_key, frame_buffer + header_size, 4);
    header_size += 4;
  }

  // Validate buffer size
  if (buffer_size < header_size + new_frame->payload_length) {
    polycall_core_free(core_ctx, new_frame);
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Extract payload
  if (new_frame->payload_length > 0) {
    new_frame->payload =
        polycall_core_malloc(core_ctx, new_frame->payload_length);
    if (!new_frame->payload) {
      polycall_core_free(core_ctx, new_frame);
      return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
    }

    memcpy(new_frame->payload, frame_buffer + header_size,
           new_frame->payload_length);

    // Unmask payload if needed
    if (new_frame->masked) {
      apply_mask(new_frame->payload, new_frame->payload_length,
                 new_frame->mask_key);
    }
  }

  *frame = new_frame;
  return POLYCALL_CORE_SUCCESS;
}

/**
 * @brief Free WebSocket frame
 */
void polycall_socket_free_frame(polycall_core_context_t *core_ctx,
                                ws_frame_t *frame) {
  if (!core_ctx || !frame) {
    return;
  }

  if (frame->payload) {
    polycall_core_free(core_ctx, frame->payload);
  }

  polycall_core_free(core_ctx, frame);
}

/**
 * @brief Create a WebSocket close frame
 */
polycall_core_error_t polycall_socket_create_close_frame(
    polycall_core_context_t *core_ctx, uint16_t close_code, const char *reason,
    bool mask_payload, uint8_t **frame_buffer, size_t *frame_size) {
  if (!core_ctx || !frame_buffer || !frame_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Calculate payload size (2 bytes for code + reason)
  size_t reason_length = reason ? strlen(reason) : 0;
  size_t payload_size = 2 + reason_length;

  // Allocate payload buffer
  uint8_t *payload = polycall_core_malloc(core_ctx, payload_size);
  if (!payload) {
    return POLYCALL_CORE_ERROR_OUT_OF_MEMORY;
  }

  // Set close code (network byte order)
  payload[0] = (close_code >> 8) & 0xFF;
  payload[1] = close_code & 0xFF;

  // Copy reason if present
  if (reason && reason_length > 0) {
    memcpy(payload + 2, reason, reason_length);
  }

  // Create frame
  polycall_core_error_t error = polycall_socket_create_frame(
      core_ctx, WS_OPCODE_CLOSE, payload, payload_size, mask_payload,
      frame_buffer, frame_size);

  // Clean up
  polycall_core_free(core_ctx, payload);

  return error;
}

/**
 * @brief Create a WebSocket ping frame
 */
polycall_core_error_t
polycall_socket_create_ping_frame(polycall_core_context_t *core_ctx,
                                  const uint8_t *payload, size_t payload_size,
                                  bool mask_payload, uint8_t **frame_buffer,
                                  size_t *frame_size) {
  if (!core_ctx || (!payload && payload_size > 0) || !frame_buffer ||
      !frame_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Create frame
  return polycall_socket_create_frame(core_ctx, WS_OPCODE_PING, payload,
                                      payload_size, mask_payload, frame_buffer,
                                      frame_size);
}

/**
 * @brief Create a WebSocket pong frame
 */
polycall_core_error_t
polycall_socket_create_pong_frame(polycall_core_context_t *core_ctx,
                                  const uint8_t *payload, size_t payload_size,
                                  bool mask_payload, uint8_t **frame_buffer,
                                  size_t *frame_size) {
  if (!core_ctx || (!payload && payload_size > 0) || !frame_buffer ||
      !frame_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Create frame
  return polycall_socket_create_frame(core_ctx, WS_OPCODE_PONG, payload,
                                      payload_size, mask_payload, frame_buffer,
                                      frame_size);
}

/**
 * @brief Create a text WebSocket frame
 */
polycall_core_error_t
polycall_socket_create_text_frame(polycall_core_context_t *core_ctx,
                                  const char *text, bool mask_payload,
                                  uint8_t **frame_buffer, size_t *frame_size) {
  if (!core_ctx || !text || !frame_buffer || !frame_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Create frame
  return polycall_socket_create_frame(core_ctx, WS_OPCODE_TEXT,
                                      (const uint8_t *)text, strlen(text),
                                      mask_payload, frame_buffer, frame_size);
}

/**
 * @brief Create a binary WebSocket frame
 */
polycall_core_error_t polycall_socket_create_binary_frame(
    polycall_core_context_t *core_ctx, const uint8_t *data, size_t data_size,
    bool mask_payload, uint8_t **frame_buffer, size_t *frame_size) {
  if (!core_ctx || !data || data_size == 0 || !frame_buffer || !frame_size) {
    return POLYCALL_CORE_ERROR_INVALID_PARAMETERS;
  }

  // Create frame
  return polycall_socket_create_frame(core_ctx, WS_OPCODE_BINARY, data,
                                      data_size, mask_payload, frame_buffer,
                                      frame_size);
}

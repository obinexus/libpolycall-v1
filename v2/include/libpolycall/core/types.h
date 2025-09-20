/* libpolycall/core/types.h - OBINexus v2 Type System */

#ifndef LIBPOLYCALL_CORE_TYPES_H
#define LIBPOLYCALL_CORE_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Status codes */
typedef enum {
    POLYCALL_SUCCESS = 0,
    POLYCALL_ERROR_INVALID_PARAMETERS = -1,
    POLYCALL_ERROR_OUT_OF_MEMORY = -2,
    POLYCALL_ERROR_NOT_INITIALIZED = -3,
    POLYCALL_ERROR_ALREADY_INITIALIZED = -4,
    POLYCALL_ERROR_NETWORK = -5,
    POLYCALL_ERROR_TIMEOUT = -6,
    POLYCALL_ERROR_UNKNOWN = -999
} polycall_status_t;

/* Configuration structure */
typedef struct polycall_config {
    size_t memory_pool_size;
    unsigned int flags;
    void* user_data;
    uint16_t port;
    const char* bind_address;
} polycall_config_t;

/* Network constants */
#define NET_TCP 1
#define NET_UDP 2
#define NET_SERVER 1
#define NET_CLIENT 2
#define NET_MAX_CLIENTS 128
#define NET_BUFFER_SIZE 4096

/* Network endpoint structure */
typedef struct network_endpoint {
    int socket_fd;
    int role;
    int protocol;
    uint16_t port;
    struct sockaddr_in addr;
    pthread_mutex_t lock;
    bool is_connected;
    char buffer[NET_BUFFER_SIZE];
} NetworkEndpoint;

/* Client state for network connections */
typedef struct client_state {
    int client_fd;
    struct sockaddr_in client_addr;
    pthread_t thread_id;
    bool active;
    void* user_data;
} ClientState;

/* Micro-service isolation structure */
typedef struct micro_isolation {
    uint32_t isolation_id;
    uint16_t source_port;
    uint16_t dest_port;
    bool active;
} micro_isolation_t;

/* Banking service structures */
typedef struct credit_score {
    uint32_t score;
    char rating[16];
} credit_score_t;

/* Module initialization flags */
#define POLYCALL_FLAG_ASYNC        0x01
#define POLYCALL_FLAG_NO_THREADS   0x02
#define POLYCALL_FLAG_DEBUG        0x04
#define POLYCALL_FLAG_ZERO_COPY    0x08
#define POLYCALL_FLAG_HOTWIRE      0x10

#endif /* LIBPOLYCALL_CORE_TYPES_H */

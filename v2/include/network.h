#ifndef NETWORK_H
#define NETWORK_H
#include <stdbool.h>
typedef struct NetworkEndpoint NetworkEndpoint;
 #define NET_TCP 1
   #define NET_UDP 2
   #define NET_SERVER 1
   #define NET_CLIENT 2
   #define NET_MAX_CLIENTS 128
   
   typedef struct network_endpoint {
       int socket_fd;
       int role;
       int protocol;
       uint16_t port;
       struct sockaddr_in addr;
       pthread_mutex_t lock;
   } NetworkEndpoint;
NetworkEndpoint* network_endpoint_create(const char* address, int port);
void network_endpoint_destroy(NetworkEndpoint* endpoint);

#endif

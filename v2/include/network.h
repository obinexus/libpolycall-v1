#ifndef NETWORK_H
#define NETWORK_H

typedef struct NetworkEndpoint NetworkEndpoint;

NetworkEndpoint* network_endpoint_create(const char* address, int port);
void network_endpoint_destroy(NetworkEndpoint* endpoint);

#endif

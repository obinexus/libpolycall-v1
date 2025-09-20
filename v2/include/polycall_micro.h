#ifndef POLYCALL_MICRO_H
#define POLYCALL_MICRO_H

#include <stdint.h>

/* Micro service definitions */
typedef struct {
    const char* service_name;
    uint16_t port;
    void* context;
} polycall_micro_service_t;

/* Micro service API */
int polycall_micro_init(polycall_micro_service_t* service);
int polycall_micro_start(polycall_micro_service_t* service);
int polycall_micro_stop(polycall_micro_service_t* service);
void polycall_micro_cleanup(polycall_micro_service_t* service);

/* Banking service specific */
int polycall_banking_init(void);
int polycall_banking_process(const void* request, void* response);
void polycall_banking_cleanup(void);

#endif /* POLYCALL_MICRO_H */

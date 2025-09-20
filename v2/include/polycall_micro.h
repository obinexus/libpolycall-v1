#ifndef POLYCALL_MICRO_H
#define POLYCALL_MICRO_H

#include "polycall.h"

typedef struct {
    polycall_context_t base;
    void* service_data;
} polycall_micro_context_t;

int polycall_micro_init(polycall_micro_context_t* ctx);
void polycall_micro_cleanup(polycall_micro_context_t* ctx);

#endif // POLYCALL_MICRO_H

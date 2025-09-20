#ifndef POLYCALL_H
#define POLYCALL_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int state;
    int thread_count;
    void* user_data;
} polycall_context_t;

// Core functions
int polycall_init(polycall_context_t* ctx);
int polycall_process(polycall_context_t* ctx, int value);
int polycall_wait(polycall_context_t* ctx, int target_state);
int polycall_cleanup(polycall_context_t* ctx);

// State machine
typedef enum {
    STATE_INIT = 0,
    STATE_READY,
    STATE_PROCESSING,
    STATE_COMPLETE,
    STATE_ERROR
} polycall_state_t;

const char* polycall_state_name(polycall_state_t state);
int polycall_transition(polycall_state_t* current, polycall_state_t next);

#endif // POLYCALL_H

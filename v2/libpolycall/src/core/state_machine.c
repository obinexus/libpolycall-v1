#include <stdlib.h>
#include <string.h>

typedef struct {
    int current_state;
    int max_states;
    void* user_data;
} state_machine_t;

void* state_machine_create(int initial_state, int max_states) {
    state_machine_t* sm = calloc(1, sizeof(state_machine_t));
    if (sm) {
        sm->current_state = initial_state;
        sm->max_states = max_states;
    }
    return sm;
}

void state_machine_destroy(void* sm) {
    free(sm);
}

int state_machine_transition(void* sm, int new_state) {
    if (!sm) return -1;
    
    state_machine_t* machine = (state_machine_t*)sm;
    if (new_state < 0 || new_state >= machine->max_states) {
        return -1;
    }
    
    machine->current_state = new_state;
    return 0;
}

int state_machine_get_state(void* sm) {
    return sm ? ((state_machine_t*)sm)->current_state : -1;
}

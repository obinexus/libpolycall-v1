#include "libpolycall/micro/polycall_micro.h"
#include <stdlib.h>

typedef struct {
    void* state;
    void* user_data;
} state_machine_impl_t;

polycall_sm_status_t polycall_sm_create_with_integrity(
    void* user_data,
    polycall_sm_t* sm,
    void* integrity_check
) {
    if (!sm) return POLYCALL_SM_ERROR;
    
    state_machine_impl_t* impl = malloc(sizeof(state_machine_impl_t));
    if (!impl) return POLYCALL_SM_ERROR;
    
    impl->state = malloc(256);
    if (!impl->state) {
        free(impl);
        return POLYCALL_SM_ERROR;
    }
    
    impl->user_data = user_data;
    *sm = (polycall_sm_t)impl;
    
    return POLYCALL_SM_SUCCESS;
}

void polycall_sm_destroy(polycall_sm_t sm) {
    if (sm) {
        state_machine_impl_t* impl = (state_machine_impl_t*)sm;
        if (impl->state) free(impl->state);
        free(impl);
    }
}

#ifndef POLYCALL_STATE_MACHINE_H
#define POLYCALL_STATE_MACHINE_H

typedef struct PolyCall_StateMachine PolyCall_StateMachine;

PolyCall_StateMachine* polycall_state_machine_create(void);
void polycall_state_machine_destroy(PolyCall_StateMachine* sm);

#endif

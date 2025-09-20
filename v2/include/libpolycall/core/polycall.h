#ifndef LIBPOLYCALL_CORE_POLYCALL_H
#define LIBPOLYCALL_CORE_POLYCALL_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

/* Opaque context type */
typedef struct polycall_context* polycall_context_t;

/* Core API functions */
polycall_status_t polycall_init(polycall_context_t* ctx);
polycall_status_t polycall_init_with_config(polycall_context_t* ctx, const polycall_config_t* config);
void polycall_cleanup(polycall_context_t* ctx);
const char* polycall_get_version(void);
const char* polycall_get_last_error(polycall_context_t ctx);

/* Flag management */
polycall_status_t polycall_set_flag(polycall_context_t ctx, unsigned int flag);
polycall_status_t polycall_clear_flag(polycall_context_t ctx, unsigned int flag);

/* User data management */
void* polycall_get_user_data(polycall_context_t ctx);
polycall_status_t polycall_set_user_data(polycall_context_t ctx, void* data);

/* State checking */
bool polycall_is_initialized(polycall_context_t ctx);

#endif /* LIBPOLYCALL_CORE_POLYCALL_H */

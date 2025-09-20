#include "libpolycall/core/token.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    char* value;
    int type;
} token_t;

void* token_create(const char* value, int type) {
    if (!value) return NULL;
    
    token_t* token = calloc(1, sizeof(token_t));
    if (token) {
        token->value = strdup(value);
        token->type = type;
    }
    return token;
}

void token_destroy(void* token) {
    if (token) {
        token_t* t = (token_t*)token;
        free(t->value);
        free(t);
    }
}

const char* token_get_value(void* token) {
    return token ? ((token_t*)token)->value : NULL;
}

int token_get_type(void* token) {
    return token ? ((token_t*)token)->type : -1;
}

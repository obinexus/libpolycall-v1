#include "libpolycall/core/tokenizer.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    char* buffer;
    size_t position;
    size_t length;
} tokenizer_state_t;

void* tokenizer_create(const char* input) {
    if (!input) return NULL;
    
    tokenizer_state_t* state = calloc(1, sizeof(tokenizer_state_t));
    if (state) {
        state->length = strlen(input);
        state->buffer = strdup(input);
        state->position = 0;
    }
    return state;
}

void tokenizer_destroy(void* tokenizer) {
    if (tokenizer) {
        tokenizer_state_t* state = (tokenizer_state_t*)tokenizer;
        free(state->buffer);
        free(state);
    }
}

int tokenizer_next(void* tokenizer, char* token, size_t max_len) {
    if (!tokenizer || !token) return -1;
    
    tokenizer_state_t* state = (tokenizer_state_t*)tokenizer;
    if (state->position >= state->length) return 0;
    
    size_t i = 0;
    while (state->position < state->length && i < max_len - 1) {
        char c = state->buffer[state->position++];
        if (c == ' ' || c == '\t' || c == '\n') {
            if (i > 0) break;
            continue;
        }
        token[i++] = c;
    }
    token[i] = '\0';
    
    return i > 0 ? 1 : 0;
}

#include "polycall_tokenizer.h"
#include <stdlib.h>
#include <string.h>

struct PolycallTokenizer {
    const char* input;
    size_t length;
    size_t position;
    uint32_t line;
    uint32_t column;
    PolycallTokenArray tokens;
};

PolycallTokenizer* polycall_tokenizer_create(const char* input) {
    PolycallTokenizer* tokenizer = calloc(1, sizeof(PolycallTokenizer));
    if (!tokenizer) return NULL;
    
    tokenizer->line = 1;
    tokenizer->column = 1;
    tokenizer->tokens.capacity = 100;
    tokenizer->tokens.tokens = calloc(tokenizer->tokens.capacity, sizeof(PolycallToken));
    
    if (input) {
        polycall_tokenizer_set_input(tokenizer, input, strlen(input));
    }
    
    return tokenizer;
}

void polycall_tokenizer_destroy(PolycallTokenizer* tokenizer) {
    if (tokenizer) {
        free(tokenizer->tokens.tokens);
        free(tokenizer);
    }
}

bool polycall_tokenizer_set_input(PolycallTokenizer* tokenizer, const char* input, size_t length) {
    if (!tokenizer || !input) return false;
    
    tokenizer->input = input;
    tokenizer->length = length;
    tokenizer->position = 0;
    tokenizer->line = 1;
    tokenizer->column = 1;
    tokenizer->tokens.count = 0;
    
    return true;
}

bool polycall_tokenizer_process(PolycallTokenizer* tokenizer, const TokenizerOperations* ops) {
    if (!tokenizer || !tokenizer->input) return false;
    
    // Simple tokenization - just create EOF token for now
    if (tokenizer->tokens.count == 0) {
        PolycallToken eof_token = {
            .type = TOKEN_EOF,
            .line = tokenizer->line,
            .column = tokenizer->column
        };
        
        if (tokenizer->tokens.count < tokenizer->tokens.capacity) {
            tokenizer->tokens.tokens[tokenizer->tokens.count++] = eof_token;
        }
        
        if (ops && ops->on_token) {
            ops->on_token(&eof_token, ops->user_data);
        }
    }
    
    return true;
}

const PolycallTokenArray* polycall_tokenizer_get_tokens(const PolycallTokenizer* tokenizer) {
    return tokenizer ? &tokenizer->tokens : NULL;
}

#ifndef POLYCALL_TOKENIZER_H
#define POLYCALL_TOKENIZER_H

#include "polycall_token.h"
#include <stddef.h>
#include <stdbool.h>

// Forward declaration
typedef struct PolycallTokenizer PolycallTokenizer;

// Tokenizer operations
typedef struct {
    void (*on_token)(const PolycallToken* token, void* user_data);
    void (*on_error)(const char* message, uint32_t line, uint32_t column, void* user_data);
    void* user_data;
} TokenizerOperations;

// Tokenizer functions
PolycallTokenizer* polycall_tokenizer_create(const char* input);
void polycall_tokenizer_destroy(PolycallTokenizer* tokenizer);
bool polycall_tokenizer_set_input(PolycallTokenizer* tokenizer, const char* input, size_t length);
bool polycall_tokenizer_process(PolycallTokenizer* tokenizer, const TokenizerOperations* ops);
const PolycallTokenArray* polycall_tokenizer_get_tokens(const PolycallTokenizer* tokenizer);

#endif // POLYCALL_TOKENIZER_H

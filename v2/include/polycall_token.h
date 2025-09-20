#ifndef POLYCALL_TOKEN_H
#define POLYCALL_TOKEN_H

#include <stdint.h>
#include <stdbool.h>

// Token types
typedef enum {
    TOKEN_EOF           = 0,
    TOKEN_NUMBER        = 1,
    TOKEN_STRING        = 2,
    TOKEN_IDENTIFIER    = 3,
    TOKEN_KEYWORD       = 4,
    TOKEN_OPERATOR      = 5,
    TOKEN_SEPARATOR     = 6,
    TOKEN_COMMENT       = 7,
    TOKEN_WHITESPACE    = 8,
    TOKEN_ERROR         = 9
} PolycallTokenType;

// Value union for tokens
typedef union PolycallValue {
    int64_t integer;
    double floating;
    const char* string;
    bool boolean;
    void* pointer;
    uint64_t raw;
} PolycallValue;

// Token structure
typedef struct {
    PolycallTokenType type;
    PolycallValue value;
    const char* text;
    size_t length;
    uint32_t line;
    uint32_t column;
} PolycallToken;

// Token array
typedef struct {
    PolycallToken* tokens;
    size_t count;
    size_t capacity;
} PolycallTokenArray;

#endif // POLYCALL_TOKEN_H

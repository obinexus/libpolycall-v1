#include "polycall_token.h"
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Array lifecycle
 * ------------------------------------------------------------------------- */

PolycallTokenArray* polycall_token_create_array(uint32_t capacity) {
    if (capacity == 0) capacity = 16;

    PolycallTokenArray* array = calloc(1, sizeof(PolycallTokenArray));
    if (!array) return NULL;

    array->tokens = calloc(capacity, sizeof(PolycallToken));
    if (!array->tokens) {
        free(array);
        return NULL;
    }

    array->capacity     = capacity;
    array->count        = 0;
    array->error_count  = 0;
    array->position.line   = 1;
    array->position.column = 1;

    return array;
}

void polycall_token_destroy_array(PolycallTokenArray* array) {
    if (!array) return;

    /* Release any heap-backed string values */
    for (uint32_t i = 0; i < array->count; i++) {
        polycall_value_destroy(&array->tokens[i].value);
    }

    free(array->tokens);
    free(array);
}

/* ---------------------------------------------------------------------------
 * Point-free token operations
 * ------------------------------------------------------------------------- */

PolycallToken polycall_token_map(const PolycallToken* token, TokenOperation op) {
    if (!token || !op) {
        PolycallToken invalid = {0};
        invalid.type  = TOKEN_INVALID;
        invalid.flags = TOKEN_FLAG_ERROR;
        return invalid;
    }
    return op(token);
}

PolycallTokenArray* polycall_token_filter(const PolycallTokenArray* array,
                                          TokenPredicate pred) {
    if (!array || !pred) return NULL;

    PolycallTokenArray* result = polycall_token_create_array(array->capacity);
    if (!result) return NULL;

    for (uint32_t i = 0; i < array->count; i++) {
        if (pred(&array->tokens[i])) {
            if (result->count >= result->capacity) {
                uint32_t new_cap = result->capacity * 2;
                PolycallToken* tmp = realloc(result->tokens,
                                             new_cap * sizeof(PolycallToken));
                if (!tmp) {
                    polycall_token_destroy_array(result);
                    return NULL;
                }
                result->tokens   = tmp;
                result->capacity = new_cap;
            }
            result->tokens[result->count++] = array->tokens[i];
        }
    }
    return result;
}

PolycallTokenArray* polycall_token_chain(const PolycallTokenArray*       array,
                                         const PolycallTokenOperations*   ops) {
    if (!array || !ops || ops->count == 0) return NULL;

    PolycallTokenArray* result = polycall_token_create_array(array->capacity);
    if (!result) return NULL;

    for (uint32_t i = 0; i < array->count; i++) {
        PolycallToken current = array->tokens[i];

        for (uint32_t j = 0; j < ops->count; j++) {
            if (ops->operations[j]) {
                current = ops->operations[j](&current);
            }
        }

        if (result->count >= result->capacity) {
            uint32_t new_cap = result->capacity * 2;
            PolycallToken* tmp = realloc(result->tokens,
                                         new_cap * sizeof(PolycallToken));
            if (!tmp) {
                polycall_token_destroy_array(result);
                return NULL;
            }
            result->tokens   = tmp;
            result->capacity = new_cap;
        }
        result->tokens[result->count++] = current;
    }
    return result;
}

/* ---------------------------------------------------------------------------
 * Value operations
 * ------------------------------------------------------------------------- */

PolycallValue polycall_value_create(PolycallValueType type, const void* data) {
    PolycallValue v = {0};
    v.type = type;

    if (!data) return v;

    switch (type) {
        case VALUE_INTEGER:
            v.data.int_value = *(const int64_t*)data;
            break;
        case VALUE_FLOAT:
            v.data.float_value = *(const double*)data;
            break;
        case VALUE_STRING:
        case VALUE_IDENTIFIER: {
            const char* src = (const char*)data;
            uint32_t    len = (uint32_t)strlen(src);
            char*       buf = malloc(len + 1);
            if (buf) {
                memcpy(buf, src, len + 1);
                v.data.string_value.data   = buf;
                v.data.string_value.length = len;
            } else {
                v.type = VALUE_NONE;
            }
            break;
        }
        default:
            break;
    }
    return v;
}

void polycall_value_destroy(PolycallValue* value) {
    if (!value) return;
    if (value->type == VALUE_STRING || value->type == VALUE_IDENTIFIER) {
        /* Cast away const: we own this allocation from polycall_value_create */
        free((void*)value->data.string_value.data);
        value->data.string_value.data   = NULL;
        value->data.string_value.length = 0;
    }
    value->type = VALUE_NONE;
}

bool polycall_value_equals(const PolycallValue* a, const PolycallValue* b) {
    if (!a || !b)          return false;
    if (a->type != b->type) return false;

    switch (a->type) {
        case VALUE_INTEGER:
            return a->data.int_value == b->data.int_value;
        case VALUE_FLOAT:
            return a->data.float_value == b->data.float_value;
        case VALUE_STRING:
        case VALUE_IDENTIFIER:
            if (a->data.string_value.length != b->data.string_value.length)
                return false;
            return memcmp(a->data.string_value.data,
                          b->data.string_value.data,
                          a->data.string_value.length) == 0;
        case VALUE_NONE:
            return true;
        default:
            return false;
    }
}

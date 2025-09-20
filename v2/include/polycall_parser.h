#ifndef POLYCALL_PARSER_H
#define POLYCALL_PARSER_H

#include "polycall.h"

typedef struct {
    char* buffer;
    size_t size;
    size_t position;
} polycall_parser_t;

polycall_parser_t* polycall_parser_create(const char* input);
void polycall_parser_destroy(polycall_parser_t* parser);
int polycall_parser_parse(polycall_parser_t* parser);

#endif // POLYCALL_PARSER_H

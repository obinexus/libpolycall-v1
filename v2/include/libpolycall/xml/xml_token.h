/* include/libpolycall/xml/xml_token.h */
#ifndef POLYCALL_XML_TOKEN_H
#define POLYCALL_XML_TOKEN_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* XML-specific token types - pure representation */
typedef enum {
    XML_TOKEN_START_TAG     = 0x0100,  /* < */
    XML_TOKEN_END_TAG       = 0x0200,  /* </ */
    XML_TOKEN_CLOSE_TAG     = 0x0400,  /* > */
    XML_TOKEN_SELF_CLOSE    = 0x0800,  /* /> */
    XML_TOKEN_ATTRIBUTE     = 0x1000,
    XML_TOKEN_TEXT          = 0x2000,
    XML_TOKEN_CDATA         = 0x4000,
    XML_TOKEN_COMMENT       = 0x8000,
    XML_TOKEN_DOCTYPE       = 0x10000,
    XML_TOKEN_PI            = 0x20000,  /* Processing Instruction */
    XML_TOKEN_NAMESPACE     = 0x40000
} XMLTokenType;

/* XML token structure - pure data, no logic */
typedef struct {
    XMLTokenType type;
    char* value;
    size_t length;
    uint32_t line;
    uint32_t column;
    
    /* For attributes only - still just data */
    struct {
        char* name;
        char* value;
    } attr;
} XMLToken;

/* Token list for batch processing */
typedef struct {
    XMLToken* tokens;
    size_t count;
    size_t capacity;
} XMLTokenList;

/* Pure functions - no business logic */
XMLTokenList* xml_token_list_create(size_t capacity);
void xml_token_list_destroy(XMLTokenList* list);
bool xml_token_list_append(XMLTokenList* list, const XMLToken* token);

#endif /* POLYCALL_XML_TOKEN_H */

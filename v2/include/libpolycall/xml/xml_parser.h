/* include/libpolycall/xml/xml_parser.h */
#ifndef POLYCALL_XML_PARSER_H
#define POLYCALL_XML_PARSER_H

#include "xml_dom.h"
#include "xml_token.h"

/* Parser state - pure tracking, no decisions */
typedef struct {
    const char* input;
    size_t position;
    size_t length;
    uint32_t line;
    uint32_t column;
    
    /* Error info - just reporting, no handling */
    char* error_message;
    uint32_t error_line;
    uint32_t error_column;
} XMLParserState;

/* Parser context - pure data structure */
typedef struct {
    XMLParserState* state;
    XMLTokenList* tokens;
    XMLDocument* document;
    
    /* Configuration - no business rules */
    struct {
        bool preserve_whitespace;
        bool include_comments;
        bool process_namespaces;
    } options;
} XMLParser;

/* Pure parsing functions - no validation logic */
XMLParser* xml_parser_create(void);
void xml_parser_destroy(XMLParser* parser);

/* Parse operations - pure transformation */
bool xml_parser_parse_string(XMLParser* parser, const char* xml);
bool xml_parser_parse_file(XMLParser* parser, const char* filename);

/* Result access - pure getters */
XMLDocument* xml_parser_get_document(XMLParser* parser);
const char* xml_parser_get_error(XMLParser* parser);

#endif /* POLYCALL_XML_PARSER_H */

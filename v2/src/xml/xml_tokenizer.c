/* src/xml/xml_tokenizer.c */
#include "libpolycall/xml/xml_token.h"
#include <string.h>
#include <stdlib.h>

/* Pure tokenization - no business logic, just lexical analysis */
XMLTokenList* xml_tokenize(const char* input) {
    XMLTokenList* list = xml_token_list_create(256);
    size_t pos = 0;
    size_t len = strlen(input);
    
    while (pos < len) {
        XMLToken token = {0};
        
        if (input[pos] == '<') {
            if (pos + 1 < len && input[pos + 1] == '/') {
                token.type = XML_TOKEN_END_TAG;
                token.value = (char*)&input[pos];
                token.length = 2;
                pos += 2;
            } else if (pos + 3 < len && 
                      strncmp(&input[pos], "<!--", 4) == 0) {
                token.type = XML_TOKEN_COMMENT;
                /* Find comment end - pure search */
                const char* end = strstr(&input[pos], "-->");
                if (end) {
                    token.value = (char*)&input[pos];
                    token.length = (end - &input[pos]) + 3;
                    pos += token.length;
                }
            } else if (pos + 8 < len && 
                      strncmp(&input[pos], "<![CDATA[", 9) == 0) {
                token.type = XML_TOKEN_CDATA;
                /* Find CDATA end - pure search */
                const char* end = strstr(&input[pos], "]]>");
                if (end) {
                    token.value = (char*)&input[pos];
                    token.length = (end - &input[pos]) + 3;
                    pos += token.length;
                }
            } else {
                token.type = XML_TOKEN_START_TAG;
                token.value = (char*)&input[pos];
                token.length = 1;
                pos++;
            }
        } else if (input[pos] == '>') {
            token.type = XML_TOKEN_CLOSE_TAG;
            token.value = (char*)&input[pos];
            token.length = 1;
            pos++;
        } else if (input[pos] == '/' && pos + 1 < len && 
                   input[pos + 1] == '>') {
            token.type = XML_TOKEN_SELF_CLOSE;
            token.value = (char*)&input[pos];
            token.length = 2;
            pos += 2;
        } else {
            /* Text content - pure extraction */
            size_t text_start = pos;
            while (pos < len && input[pos] != '<') {
                pos++;
            }
            token.type = XML_TOKEN_TEXT;
            token.value = (char*)&input[text_start];
            token.length = pos - text_start;
        }
        
        xml_token_list_append(list, &token);
    }
    
    return list;
}

/* include/libpolycall/xml/xml_dom.h */
#ifndef POLYCALL_XML_DOM_H
#define POLYCALL_XML_DOM_H

#include "xml_token.h"

/* Pure DOM node representation - no logic */
typedef struct XMLNode XMLNode;
typedef struct XMLAttribute XMLAttribute;

struct XMLAttribute {
    char* name;
    char* value;
    char* namespace;  /* Optional namespace */
    XMLAttribute* next;
};

struct XMLNode {
    /* Node data - pure representation */
    char* name;
    char* namespace;
    char* text;
    
    /* Structure - pure tree */
    XMLNode* parent;
    XMLNode* first_child;
    XMLNode* last_child;
    XMLNode* next_sibling;
    XMLNode* prev_sibling;
    
    /* Attributes - pure list */
    XMLAttribute* attributes;
    
    /* Metadata - no decisions */
    uint32_t line;
    uint32_t column;
};

/* DOM Document - pure container */
typedef struct {
    XMLNode* root;
    char* version;
    char* encoding;
    size_t node_count;
} XMLDocument;

/* Pure construction/destruction - no validation */
XMLDocument* xml_document_create(void);
void xml_document_destroy(XMLDocument* doc);
XMLNode* xml_node_create(const char* name);
void xml_node_destroy(XMLNode* node);

/* Pure tree operations - no rules */
void xml_node_append_child(XMLNode* parent, XMLNode* child);
void xml_node_add_attribute(XMLNode* node, const char* name, const char* value);

/* Pure accessors - no logic */
XMLNode* xml_node_find_child(const XMLNode* parent, const char* name);
const char* xml_node_get_attribute(const XMLNode* node, const char* name);

#endif /* POLYCALL_XML_DOM_H */

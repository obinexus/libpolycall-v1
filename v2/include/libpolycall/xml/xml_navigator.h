/* include/libpolycall/xml/xml_navigator.h */
#ifndef POLYCALL_XML_NAVIGATOR_H
#define POLYCALL_XML_NAVIGATOR_H

#include "xml_dom.h"

/* Path expression - pure representation */
typedef struct {
    char** segments;
    size_t segment_count;
} XMLPath;

/* Navigator - pure traversal, no logic */
typedef struct {
    XMLNode* current;
    XMLDocument* document;
} XMLNavigator;

/* Pure navigation functions */
XMLNavigator* xml_navigator_create(XMLDocument* doc);
void xml_navigator_destroy(XMLNavigator* nav);

/* Pure traversal - no validation */
bool xml_navigator_goto(XMLNavigator* nav, const char* path);
XMLNode* xml_navigator_find(XMLNavigator* nav, const char* expression);

/* Collection operations - pure iteration */
typedef struct {
    XMLNode** nodes;
    size_t count;
    size_t capacity;
} XMLNodeList;

XMLNodeList* xml_navigator_find_all(XMLNavigator* nav, const char* expression);
void xml_node_list_destroy(XMLNodeList* list);

#endif /* POLYCALL_XML_NAVIGATOR_H */

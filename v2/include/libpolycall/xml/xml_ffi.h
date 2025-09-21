/* include/libpolycall/xml/xml_ffi.h */
#ifndef POLYCALL_XML_FFI_H
#define POLYCALL_XML_FFI_H

/* Pure FFI exports - no business logic */
#ifdef __cplusplus
extern "C" {
#endif

/* Parser exports */
void* polycall_xml_parser_create(void);
int polycall_xml_parser_parse(void* parser, const char* xml);
void* polycall_xml_parser_get_root(void* parser);
void polycall_xml_parser_destroy(void* parser);

/* Node navigation exports */
void* polycall_xml_node_get_child(void* node, const char* name);
const char* polycall_xml_node_get_text(void* node);
const char* polycall_xml_node_get_attr(void* node, const char* attr);

/* Config reader exports */
void* polycall_config_load(const char* path);
const char* polycall_config_get(void* config, const char* xpath);
void polycall_config_destroy(void* config);

#ifdef __cplusplus
}
#endif

#endif /* POLYCALL_XML_FFI_H */

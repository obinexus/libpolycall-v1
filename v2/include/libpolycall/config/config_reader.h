/* include/libpolycall/config/config_reader.h */
#ifndef POLYCALL_CONFIG_READER_H
#define POLYCALL_CONFIG_READER_H

#include "../xml/xml_dom.h"

/* Configuration types - pure enumeration */
typedef enum {
    CONFIG_TYPE_HOTWIRE,
    CONFIG_TYPE_SERVICE,
    CONFIG_TYPE_MODULE,
    CONFIG_TYPE_SCHEMA,
    CONFIG_TYPE_UNKNOWN
} ConfigType;

/* Config structure - pure data container */
typedef struct {
    ConfigType type;
    XMLDocument* document;
    char* path;
    
    /* Metadata - no processing */
    char* version;
    char* schema_uri;
} ConfigDocument;

/* Config set - pure collection */
typedef struct {
    ConfigDocument** configs;
    size_t count;
    size_t capacity;
} ConfigSet;

/* Pure config operations - no business logic */
ConfigDocument* config_load(const char* path);
void config_destroy(ConfigDocument* config);

/* Pure accessors - no validation */
XMLNode* config_get_section(const ConfigDocument* config, const char* path);
const char* config_get_value(const ConfigDocument* config, const char* xpath);

/* Batch operations - pure collection handling */
ConfigSet* config_set_create(void);
void config_set_destroy(ConfigSet* set);
bool config_set_add(ConfigSet* set, ConfigDocument* config);

#endif /* POLYCALL_CONFIG_READER_H */

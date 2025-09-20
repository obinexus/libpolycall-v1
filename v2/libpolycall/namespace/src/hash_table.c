#include "../include/namespace_resolver.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <regex.h>

// FNV-1a hash for O(1) lookup performance
static uint32_t fnv1a_hash(const char* key) {
    uint32_t hash = 2166136261u;
    for (const char* p = key; *p; p++) {
        hash ^= (uint32_t)*p;
        hash *= 16777619u;
    }
    return hash;
}

namespace_table_t* namespace_table_create(size_t initial_capacity) {
    namespace_table_t* table = calloc(1, sizeof(namespace_table_t));
    if (!table) return NULL;
    
    table->bucket_count = initial_capacity;
    table->buckets = calloc(initial_capacity, sizeof(module_descriptor_t*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    
    table->load_factor = 0.75;
    table->module_count = 0;
    return table;
}

void namespace_table_destroy(namespace_table_t* table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->bucket_count; i++) {
        module_descriptor_t* current = table->buckets[i];
        while (current) {
            module_descriptor_t* next = current->next;
            free(current->namespace_key);
            free(current->library_path);
            if (current->handle) {
                dlclose(current->handle);
            }
            free(current);
            current = next;
        }
    }
    free(table->buckets);
    free(table);
}

int namespace_register(namespace_table_t* table, 
                       const char* namespace_key, 
                       const char* library_path) {
    if (!table || !namespace_key || !library_path) return -1;
    
    uint32_t hash = fnv1a_hash(namespace_key);
    size_t index = hash % table->bucket_count;
    
    // Check for existing entry
    module_descriptor_t* current = table->buckets[index];
    while (current) {
        if (strcmp(current->namespace_key, namespace_key) == 0) {
            return -1; // Already registered
        }
        current = current->next;
    }
    
    // Create new descriptor
    module_descriptor_t* desc = calloc(1, sizeof(module_descriptor_t));
    if (!desc) return -1;
    
    desc->namespace_key = strdup(namespace_key);
    desc->library_path = strdup(library_path);
    desc->hash = hash;
    desc->is_loaded = false;
    desc->handle = NULL;
    
    // Insert at head for O(1) insertion
    desc->next = table->buckets[index];
    table->buckets[index] = desc;
    table->module_count++;
    
    return 0;
}

module_descriptor_t* namespace_lookup(namespace_table_t* table, 
                                      const char* namespace_key) {
    if (!table || !namespace_key) return NULL;
    
    uint32_t hash = fnv1a_hash(namespace_key);
    size_t index = hash % table->bucket_count;
    
    module_descriptor_t* current = table->buckets[index];
    while (current) {
        if (current->hash == hash && 
            strcmp(current->namespace_key, namespace_key) == 0) {
            return current; // O(1) average case
        }
        current = current->next;
    }
    return NULL;
}

void* namespace_load_module(namespace_table_t* table, 
                            const char* namespace_key) {
    module_descriptor_t* desc = namespace_lookup(table, namespace_key);
    if (!desc) return NULL;
    
    if (!desc->is_loaded && desc->library_path) {
        desc->handle = dlopen(desc->library_path, RTLD_LAZY | RTLD_LOCAL);
        if (desc->handle) {
            desc->is_loaded = true;
        }
    }
    return desc->handle;
}

int namespace_load_pattern(namespace_table_t* table, const char* pattern) {
    if (!table || !pattern) return -1;
    
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return -1;
    }
    
    int loaded_count = 0;
    for (size_t i = 0; i < table->bucket_count; i++) {
        module_descriptor_t* current = table->buckets[i];
        while (current) {
            if (regexec(&regex, current->namespace_key, 0, NULL, 0) == 0) {
                if (namespace_load_module(table, current->namespace_key)) {
                    loaded_count++;
                }
            }
            current = current->next;
        }
    }
    
    regfree(&regex);
    return loaded_count;
}

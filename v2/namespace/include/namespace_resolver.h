#ifndef LIBPOLYCALL_NAMESPACE_RESOLVER_H
#define LIBPOLYCALL_NAMESPACE_RESOLVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Module descriptor for hash table
typedef struct module_descriptor {
    char* namespace_key;           // "feature-A/component-subset-1/module-name-001"
    char* library_path;            // Path to .a/.so/.dll
    void* handle;                  // dlopen handle (if loaded)
    uint32_t hash;                 // Precomputed hash
    bool is_loaded;
    struct module_descriptor* next; // Collision chain
} module_descriptor_t;

// O(1) Hash table for namespace lookup
typedef struct namespace_table {
    module_descriptor_t** buckets;
    size_t bucket_count;
    size_t module_count;
    double load_factor;
} namespace_table_t;

// Directed Acyclic Graph for dependencies
typedef struct dag_node {
    char* module_name;
    struct dag_node** dependencies;
    size_t dep_count;
    bool visited;
    bool in_stack;
} dag_node_t;

typedef struct dag_graph {
    dag_node_t** nodes;
    size_t node_count;
    namespace_table_t* ns_table;
} dag_graph_t;

// Module loader
typedef struct polycall_loader {
    namespace_table_t* ns_table;
    dag_graph_t* dep_graph;
    char* module_path;
} polycall_loader_t;

// Public API
namespace_table_t* namespace_table_create(size_t initial_capacity);
void namespace_table_destroy(namespace_table_t* table);

// O(1) operations
int namespace_register(namespace_table_t* table, 
                       const char* namespace_key, 
                       const char* library_path);
                       
module_descriptor_t* namespace_lookup(namespace_table_t* table, 
                                      const char* namespace_key);
                                      
void* namespace_load_module(namespace_table_t* table, 
                            const char* namespace_key);

// Pattern matching
int namespace_load_pattern(namespace_table_t* table, 
                           const char* pattern);

// DAG operations
int dag_resolve_dependencies(dag_graph_t* graph, char*** load_order);
bool dag_has_cycle(dag_graph_t* graph);

// Loader API
polycall_loader_t* polycall_loader_init(const char* registry_path);
void* polycall_load(polycall_loader_t* loader, const char* namespace);
int polycall_load_pattern(polycall_loader_t* loader, const char* pattern);

#endif // LIBPOLYCALL_NAMESPACE_RESOLVER_H

#ifndef LIBPOLYCALL_DAG_RESOLVER_H
#define LIBPOLYCALL_DAG_RESOLVER_H

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

// Topological sort for dependency order
int dag_resolve_dependencies(dag_graph_t* graph, char*** load_order);

// Cycle detection
bool dag_has_cycle(dag_graph_t* graph);

#endif

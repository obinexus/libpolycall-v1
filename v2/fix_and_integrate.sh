#!/bin/bash
# fix_and_integrate.sh - Fix restructure issues and add namespace system

set -e

echo "Fixing v2 restructure and adding namespace lookup system..."

# Fix the missing internal headers
for feature in core adapter hotwire socket micro nlm stream zero; do
    mkdir -p src/$feature/internal
    if [ ! -f "src/$feature/internal/${feature}_private.h" ]; then
        cat > src/$feature/internal/${feature}_private.h << EOF
#ifndef LIBPOLYCALL_${feature^^}_PRIVATE_H
#define LIBPOLYCALL_${feature^^}_PRIVATE_H

// Private definitions for $feature module
// Internal use only - not exposed in public API

#include <stdint.h>
#include <stdbool.h>

typedef struct ${feature}_internal {
    void* private_data;
    uint32_t flags;
} ${feature}_internal_t;

#endif // LIBPOLYCALL_${feature^^}_PRIVATE_H
EOF
        echo "✓ Created src/$feature/internal/${feature}_private.h"
    fi
done

# Create namespace system directories
echo "Setting up namespace lookup system..."
mkdir -p namespace/{include,src,registry}
mkdir -p modules/feature-{A..Z}/component-subset-{1..3}
mkdir -p loaders/{linux,darwin,windows}

# Create namespace resolver header
cat > namespace/include/namespace_resolver.h << 'EOF'
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
EOF
echo "✓ Created namespace resolver header"

# Create hash table implementation
cat > namespace/src/hash_table.c << 'EOF'
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
EOF
echo "✓ Created hash table implementation"

# Create module registry XML
cat > namespace/registry/module_registry.xml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<module_registry xmlns="http://obinexus.com/schema/modules" version="2.0.0">
    <!-- Feature A: Core functionality -->
    <feature id="A" name="core" category="essential">
        <component-subset id="1" name="foundation">
            <module id="001" name="context">
                <namespace>feature-A/component-subset-1/module-name-001</namespace>
                <library>
                    <static>libpolycall_core_context.a</static>
                    <shared>libpolycall_core_context.so</shared>
                    <darwin>libpolycall_core_context.dylib</darwin>
                    <windows>polycall_core_context.dll</windows>
                </library>
                <dependencies/>
            </module>
            <module id="002" name="memory">
                <namespace>feature-A/component-subset-1/module-name-002</namespace>
                <library>
                    <static>libpolycall_core_memory.a</static>
                    <shared>libpolycall_core_memory.so</shared>
                </library>
                <dependencies>
                    <dependency>feature-A/component-subset-1/module-name-001</dependency>
                </dependencies>
            </module>
        </component-subset>
    </feature>
    
    <!-- Feature B: Adapter system -->
    <feature id="B" name="adapter" category="binding">
        <component-subset id="1" name="ffi">
            <module id="003" name="ffi_bridge">
                <namespace>feature-B/component-subset-1/module-name-003</namespace>
                <library>
                    <static>libpolycall_adapter_ffi.a</static>
                    <shared>libpolycall_adapter_ffi.so</shared>
                </library>
                <dependencies>
                    <dependency>feature-A/component-subset-1/module-name-001</dependency>
                </dependencies>
            </module>
        </component-subset>
    </feature>
    
    <!-- Feature H: Hotwire system -->
    <feature id="H" name="hotwire" category="runtime">
        <component-subset id="1" name="loader">
            <module id="010" name="hot_loader">
                <namespace>feature-H/component-subset-1/module-name-010</namespace>
                <library>
                    <static>libpolycall_hotwire_loader.a</static>
                    <shared>libpolycall_hotwire_loader.so</shared>
                </library>
                <dependencies>
                    <dependency>feature-A/component-subset-1/module-name-001</dependency>
                    <dependency>feature-B/component-subset-1/module-name-003</dependency>
                </dependencies>
            </module>
        </component-subset>
    </feature>
    
    <!-- Feature N: NLM-Atlas -->
    <feature id="N" name="nlm" category="service-discovery">
        <component-subset id="1" name="atlas">
            <module id="020" name="avl_huffman">
                <namespace>feature-N/component-subset-1/module-name-020</namespace>
                <library>
                    <static>libpolycall_nlm_atlas.a</static>
                    <shared>libpolycall_nlm_atlas.so</shared>
                </library>
                <dependencies>
                    <dependency>feature-A/component-subset-1/module-name-001</dependency>
                </dependencies>
            </module>
        </component-subset>
    </feature>
</module_registry>
EOF
echo "✓ Created module registry"

# Create example module generator
cat > scripts/generate_module.sh << 'EOF'
#!/bin/bash
# Generate a module following the namespace pattern

if [ $# -ne 3 ]; then
    echo "Usage: $0 <feature-letter> <component-number> <module-number>"
    echo "Example: $0 A 1 001"
    exit 1
fi

FEATURE=$1
COMPONENT=$2
MODULE=$(printf "%03d" $3)

NAMESPACE="feature-${FEATURE}/component-subset-${COMPONENT}/module-name-${MODULE}"
MODULE_DIR="modules/${NAMESPACE}"
INCLUDE_DIR="include/libpolycall/${NAMESPACE}"

# Create directories
mkdir -p "$MODULE_DIR"
mkdir -p "$INCLUDE_DIR"

# Generate header file
cat > "${INCLUDE_DIR}.h" << EOH
#ifndef LIBPOLYCALL_FEATURE_${FEATURE}_COMPONENT_${COMPONENT}_MODULE_${MODULE}_H
#define LIBPOLYCALL_FEATURE_${FEATURE}_COMPONENT_${COMPONENT}_MODULE_${MODULE}_H

#ifdef __cplusplus
extern "C" {
#endif

// Module initialization
int module_${MODULE}_init(void);

// Module cleanup
void module_${MODULE}_cleanup(void);

// Module-specific functionality
void* module_${MODULE}_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif
EOH

# Generate source file
cat > "${MODULE_DIR}.c" << EOC
#include <libpolycall/${NAMESPACE}.h>
#include <stdlib.h>
#include <stdio.h>

static struct {
    int initialized;
    void* private_data;
} module_state = {0};

int module_${MODULE}_init(void) {
    if (module_state.initialized) {
        return 0; // Already initialized
    }
    
    printf("Initializing module: ${NAMESPACE}\n");
    module_state.private_data = calloc(1, 1024);
    module_state.initialized = 1;
    
    return 0;
}

void module_${MODULE}_cleanup(void) {
    if (!module_state.initialized) {
        return;
    }
    
    free(module_state.private_data);
    module_state.private_data = NULL;
    module_state.initialized = 0;
}

void* module_${MODULE}_get_interface(void) {
    return module_state.private_data;
}
EOC

echo "Generated module: ${NAMESPACE}"
echo "  Header: ${INCLUDE_DIR}.h"
echo "  Source: ${MODULE_DIR}.c"
EOF
chmod +x scripts/generate_module.sh

echo "✓ Created module generator"

# Create a sample set of modules
echo "Generating sample modules..."
for feature_component_module in "A 1 001" "A 1 002" "B 1 003" "H 1 010" "N 1 020"; do
    scripts/generate_module.sh $feature_component_module
done

# Update main Makefile to include namespace system
cat >> Makefile << 'EOF'

# ============================================
# Namespace System Rules
# ============================================

namespace: directories
	@echo "Building namespace resolver..."
	@$(CC) $(CFLAGS) -c namespace/src/hash_table.c -o $(OBJ_DIR)/hash_table.o
	@$(AR) rcs $(STATIC_DIR)/libpolycall_namespace.a $(OBJ_DIR)/hash_table.o
	@$(CC) -shared -o $(SHARED_DIR)/libpolycall_namespace.so $(OBJ_DIR)/hash_table.o
	@echo "✓ Namespace system built"

# Build modules matching pattern
modules-pattern-%:
	@pattern=$$(echo $* | sed 's/-/\//g'); \
	for module in $$(find modules -path "*$$pattern*" -name "*.c"); do \
		obj=$$(echo $$module | sed 's/\.c/.o/'); \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$module -o $$obj; \
		echo "✓ Built $$module"; \
	done

# Build all modules
modules-all:
	@find modules -name "*.c" | while read module; do \
		obj=$$(echo $$module | sed 's/\.c/.o/'); \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$module -o $$obj; \
		base=$$(basename $$module .c); \
		dir=$$(dirname $$module); \
		$(AR) rcs $$dir/$$base.a $$obj; \
		$(CC) -shared -o $$dir/$$base.so $$obj $(LDFLAGS); \
		echo "✓ Built $$base (.a/.so)"; \
	done

.PHONY: namespace modules-pattern-% modules-all
EOF

echo "✓ Updated Makefile with namespace rules"

# Create test program
cat > test_namespace.c << 'EOF'
#include <stdio.h>
#include <assert.h>
#include "namespace/include/namespace_resolver.h"

int main() {
    printf("Testing namespace lookup system...\n");
    
    // Create namespace table
    namespace_table_t* table = namespace_table_create(64);
    assert(table != NULL);
    
    // Register some modules
    namespace_register(table, 
        "feature-A/component-subset-1/module-name-001",
        "modules/feature-A/component-subset-1/module-name-001.so");
    
    namespace_register(table,
        "feature-B/component-subset-1/module-name-003", 
        "modules/feature-B/component-subset-1/module-name-003.so");
    
    // Test O(1) lookup
    module_descriptor_t* desc = namespace_lookup(table,
        "feature-A/component-subset-1/module-name-001");
    assert(desc != NULL);
    printf("✓ O(1) lookup successful\n");
    
    // Test pattern loading
    int loaded = namespace_load_pattern(table, 
        "feature-[AB]/component-subset-1/module-name-.*");
    printf("✓ Pattern matched %d modules\n", loaded);
    
    namespace_table_destroy(table);
    printf("✓ All tests passed!\n");
    
    return 0;
}
EOF

echo ""
echo "========================================="
echo "✓ Fix and integration complete!"
echo "========================================="
echo ""
echo "Namespace system structure:"
echo "  namespace/         - Hash table O(1) lookup implementation"
echo "  modules/          - Feature-isolated modules"
echo "  loaders/          - Platform-specific loaders"
echo ""
echo "Next steps:"
echo "  1. Build namespace system: make namespace"
echo "  2. Generate modules: scripts/generate_module.sh <feature> <component> <number>"
echo "  3. Build all modules: make modules-all"
echo "  4. Test namespace: gcc test_namespace.c -o test_namespace -L. -lpolycall_namespace -ldl"
echo ""
echo "Module include pattern:"
echo "  #include <libpolycall/feature-[A-Z]/component-subset-[0-9]/module-name-NNN>"
echo ""
echo "Example usage:"
echo "  polycall_load(loader, \"feature-A/component-subset-1/module-name-001\");"
echo "  polycall_load_pattern(loader, \"feature-[AB]/.*\");"

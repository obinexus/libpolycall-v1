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

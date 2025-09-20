#!/bin/bash
# OBINexus libpolycall v2 - Type System Fix
# Resolves conflicting definitions and structures module headers

set -e

echo "=== OBINexus Type System and Header Fix ==="
echo "Resolving type conflicts and structuring modules..."

# 1. Backup and consolidate headers
echo "[1/6] Consolidating headers..."
mkdir -p include/libpolycall/core.backup
cp -r include/libpolycall/core/* include/libpolycall/core.backup/ 2>/dev/null || true

# 2. Create the master polycall.h with proper types
echo "[2/6] Creating unified type definitions..."
cat > include/libpolycall/core/polycall.h << 'EOF'
#ifndef LIBPOLYCALL_CORE_POLYCALL_H
#define LIBPOLYCALL_CORE_POLYCALL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration for opaque pointer */
struct polycall_context;

/* Opaque context type - this is a POINTER */
typedef struct polycall_context* polycall_context_t;

/* Status codes */
typedef enum {
    POLYCALL_SUCCESS = 0,
    POLYCALL_ERROR_INVALID_PARAMETERS = -1,
    POLYCALL_ERROR_OUT_OF_MEMORY = -2,
    POLYCALL_ERROR_NOT_INITIALIZED = -3,
    POLYCALL_ERROR_ALREADY_INITIALIZED = -4,
    POLYCALL_ERROR_INVALID_STATE = -5,
    POLYCALL_ERROR_TIMEOUT = -6,
    POLYCALL_ERROR_UNKNOWN = -99
} polycall_status_t;

/* Configuration structure */
typedef struct {
    unsigned int flags;
    size_t memory_pool_size;
    void* user_data;
} polycall_config_t;

/* Core API functions */
polycall_status_t polycall_init_with_config(
    polycall_context_t* ctx, 
    const polycall_config_t* config
);

void polycall_cleanup(polycall_context_t ctx);
const char* polycall_get_version(void);
const char* polycall_get_last_error(polycall_context_t ctx);

#ifdef __cplusplus
}
#endif

#endif /* LIBPOLYCALL_CORE_POLYCALL_H */
EOF

# 3. Fix polycall.c to match the header
echo "[3/6] Fixing polycall.c implementation..."
cat > src/core/polycall.c << 'EOF'
#include "libpolycall/core/polycall.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define POLYCALL_VERSION "2.0.0"
#define MAX_ERROR_LENGTH 256

/* Internal context structure - hidden from API users */
struct polycall_context {
    char last_error[MAX_ERROR_LENGTH];
    void* user_data;
    size_t memory_pool_size;
    unsigned int flags;
    bool is_initialized;
};

/* Static helper functions */
static void set_error(polycall_context_t ctx, const char* error) {
    if (ctx && error) {
        strncpy(ctx->last_error, error, MAX_ERROR_LENGTH - 1);
        ctx->last_error[MAX_ERROR_LENGTH - 1] = '\0';
    }
}

/* API Implementation */

polycall_status_t polycall_init_with_config(
    polycall_context_t* ctx, 
    const polycall_config_t* config
) {
    if (!ctx) {
        return POLYCALL_ERROR_INVALID_PARAMETERS;
    }

    /* Allocate context */
    struct polycall_context* new_ctx = calloc(1, sizeof(struct polycall_context));
    if (!new_ctx) {
        return POLYCALL_ERROR_OUT_OF_MEMORY;
    }

    /* Initialize with defaults */
    new_ctx->memory_pool_size = 1024 * 1024; /* 1MB default */
    new_ctx->flags = 0;

    /* Apply configuration if provided */
    if (config) {
        new_ctx->flags = config->flags;
        if (config->memory_pool_size > 0) {
            new_ctx->memory_pool_size = config->memory_pool_size;
        }
        new_ctx->user_data = config->user_data;
    }

    /* Mark as initialized */
    new_ctx->is_initialized = true;
    new_ctx->last_error[0] = '\0';

    *ctx = new_ctx;
    return POLYCALL_SUCCESS;
}

void polycall_cleanup(polycall_context_t ctx) {
    if (ctx && ctx->is_initialized) {
        ctx->is_initialized = false;
        free(ctx);
    }
}

const char* polycall_get_version(void) {
    return POLYCALL_VERSION;
}

const char* polycall_get_last_error(polycall_context_t ctx) {
    if (!ctx) {
        return "Invalid context";
    }
    return ctx->last_error;
}
EOF

# 4. Generate module headers for feature modules
echo "[4/6] Generating module header structure..."

# Create module header template generator
generate_module_header() {
    local feature=$1
    local module_num=$2
    local module_name="module-name-$(printf "%03d" $module_num)"
    
    mkdir -p "include/libpolycall/${feature}/component-subset-1"
    
    cat > "include/libpolycall/${feature}/component-subset-1/${module_name}.h" << EOF
#ifndef LIBPOLYCALL_${feature^^}_${module_name^^}_H
#define LIBPOLYCALL_${feature^^}_${module_name^^}_H

#include "libpolycall/core/polycall.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Module: ${module_name} in feature ${feature} */

/* Module-specific types */
typedef struct {
    polycall_context_t context;
    void* module_data;
    uint32_t module_id;
} ${module_name}_handle_t;

/* Module initialization */
polycall_status_t ${module_name}_init(
    polycall_context_t ctx,
    ${module_name}_handle_t** handle
);

/* Module operations */
polycall_status_t ${module_name}_process(
    ${module_name}_handle_t* handle,
    const void* input,
    void* output
);

/* Module cleanup */
void ${module_name}_cleanup(${module_name}_handle_t* handle);

/* Module version */
const char* ${module_name}_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* LIBPOLYCALL_${feature^^}_${module_name^^}_H */
EOF
}

# Generate headers for existing modules
generate_module_header "feature-A" 1
generate_module_header "feature-A" 2
generate_module_header "feature-B" 3
generate_module_header "feature-H" 8
generate_module_header "feature-N" 16

# 5. Create module schema header
echo "[5/6] Creating module schema definitions..."
cat > include/libpolycall/module_schema.h << 'EOF'
#ifndef LIBPOLYCALL_MODULE_SCHEMA_H
#define LIBPOLYCALL_MODULE_SCHEMA_H

#include <stdint.h>

/* Module schema version */
#define MODULE_SCHEMA_VERSION "1.0.0"

/* Module types */
typedef enum {
    MODULE_TYPE_CORE = 0x0001,
    MODULE_TYPE_ADAPTER = 0x0002,
    MODULE_TYPE_NETWORK = 0x0004,
    MODULE_TYPE_CRYPTO = 0x0008,
    MODULE_TYPE_STORAGE = 0x0010,
    MODULE_TYPE_COMPUTE = 0x0020,
    MODULE_TYPE_CUSTOM = 0x8000
} module_type_t;

/* Module capability flags */
typedef enum {
    MODULE_CAP_ASYNC = 0x0001,
    MODULE_CAP_THREAD_SAFE = 0x0002,
    MODULE_CAP_STATELESS = 0x0004,
    MODULE_CAP_REALTIME = 0x0008,
    MODULE_CAP_STREAMING = 0x0010
} module_capability_t;

/* Module metadata */
typedef struct {
    char name[64];
    char version[32];
    module_type_t type;
    uint32_t capabilities;
    uint32_t required_memory;
    uint32_t max_instances;
} module_metadata_t;

/* Module registry entry */
typedef struct {
    uint32_t module_id;
    module_metadata_t metadata;
    void* (*create_func)(void);
    void (*destroy_func)(void*);
    int (*init_func)(void*, void*);
    int (*process_func)(void*, const void*, void*);
} module_registry_entry_t;

/* Module loader functions */
int module_schema_register(const module_registry_entry_t* entry);
int module_schema_unregister(uint32_t module_id);
const module_registry_entry_t* module_schema_get(uint32_t module_id);
int module_schema_enumerate(module_registry_entry_t** entries, size_t* count);

#endif /* LIBPOLYCALL_MODULE_SCHEMA_H */
EOF

# 6. Create clean build Makefile
echo "[6/6] Creating clean build configuration..."
cat > Makefile.clean << 'EOF'
# OBINexus libpolycall v2 - Clean Build
# Type-safe compilation

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pthread -fPIC
CFLAGS += -I. -I./include -I./include/libpolycall
LDFLAGS = -pthread

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib

# Core sources only (no duplicates, no CMake files)
SOURCES = src/core/polycall.c \
          src/core/tokenizer.c \
          src/core/token.c \
          src/core/state_machine.c \
          src/adapter/polycall_dop_adapter.c \
          src/socket/polycall_socket.c \
          src/socket/network.c

# Filter out non-existent files
EXISTING_SOURCES = $(wildcard $(SOURCES))

# Object files
OBJECTS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(EXISTING_SOURCES))

# Targets
STATIC_LIB = $(LIB_DIR)/libpolycall.a
SHARED_LIB = $(LIB_DIR)/libpolycall.so

all: directories $(STATIC_LIB) $(SHARED_LIB)

directories:
	@mkdir -p $(OBJ_DIR)/core $(OBJ_DIR)/adapter $(OBJ_DIR)/socket
	@mkdir -p $(LIB_DIR)

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(STATIC_LIB): $(OBJECTS)
	@echo "AR $@"
	@ar rcs $@ $(OBJECTS)

$(SHARED_LIB): $(OBJECTS)
	@echo "LD $@"
	@$(CC) -shared -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	@rm -rf $(BUILD_DIR)

verify:
	@echo "=== Type System Verification ==="
	@echo "Checking header consistency..."
	@grep -h "polycall_context_t" include/libpolycall/core/polycall.h
	@echo ""
	@echo "Library outputs:"
	@ls -la $(LIB_DIR)/ 2>/dev/null || echo "Not built yet"

.PHONY: all clean directories verify
EOF

echo "=== Fix Complete ==="
echo ""
echo "Changes made:"
echo "  ✓ Unified type system with opaque pointer"
echo "  ✓ Fixed polycall_context_t to be a pointer type"
echo "  ✓ Added all missing error constants"
echo "  ✓ Generated module header structure"
echo "  ✓ Created module schema definitions"
echo "  ✓ Clean build configuration"
echo ""
echo "Build with:"
echo "  make -f Makefile.clean"
echo ""
echo "Module headers created in:"
echo "  include/libpolycall/feature-*/component-subset-1/module-name-*.h"

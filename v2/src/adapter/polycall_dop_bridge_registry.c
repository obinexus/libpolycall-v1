/* Standard library includes */
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Core types */
#include "libpolycall/core/types.h"

/**
 * @file polycall_dop_bridge_registry.c
 * @brief DOP Adapter Language Bridge Registry Implementation
 *
 * LibPolyCall DOP Adapter Framework - Bridge Management System
 * OBINexus Computing - Aegis Project Technical Infrastructure
 *
 * Implements centralized registry for language bridges, providing registration,
 * discovery, and lifecycle management for cross-language adapters.
 * Essential for managing multiple language runtimes in banking applications.
 *
 * @version 1.0.0
 * @date 2025-06-09
 */

#include "polycall/core/dop/polycall_dop_adapter.h"
#include "polycall/core/polycall_core.h"
#include "polycall/core/polycall_memory.h"

#include <assert.h>
#include <dlfcn.h> // For dynamic loading
#include <string.h>

/* ====================================================================
 * Bridge Registry Internal Structures
 * ==================================================================== */

/**
 * @brief Language bridge implementation structure
 */
struct polycall_dop_bridge {
  polycall_dop_language_t language; ///< Target language
  const char *bridge_name;          ///< Bridge identifier
  const char *version;              ///< Bridge version

  /* Bridge lifecycle callbacks */
  polycall_dop_error_t (*initialize)(
      polycall_dop_bridge_t *bridge,
      const polycall_dop_component_config_t *config);

  polycall_dop_error_t (*create_component)(
      polycall_dop_bridge_t *bridge,
      const polycall_dop_component_config_t *config,
      polycall_dop_component_t *component);

  polycall_dop_error_t (*invoke_method)(
      polycall_dop_bridge_t *bridge, polycall_dop_component_t *component,
      const polycall_dop_invocation_t *invocation,
      polycall_dop_result_t *result);

  polycall_dop_error_t (*destroy_component)(
      polycall_dop_bridge_t *bridge, polycall_dop_component_t *component);

  polycall_dop_error_t (*cleanup)(polycall_dop_bridge_t *bridge);

  /* Type conversion functions */
  polycall_dop_error_t (*convert_to_native)(
      polycall_dop_bridge_t *bridge, const polycall_dop_value_t *dop_value,
      void **native_value);

  polycall_dop_error_t (*convert_from_native)(polycall_dop_bridge_t *bridge,
                                              const void *native_value,
                                              polycall_dop_value_t *dop_value);

  /* Bridge state and context */
  void *runtime_context; ///< Language-specific context
  void *bridge_handle;   ///< Dynamic library handle
  bool is_initialized;   ///< Initialization state

  /* Statistics */
  uint64_t components_created;      ///< Number of components created
  uint64_t total_invocations;       ///< Total method invocations
  uint64_t total_execution_time_ns; ///< Total execution time
  uint64_t errors_encountered;      ///< Error count
};

/**
 * @brief Bridge registry structure
 */
typedef struct {
  polycall_dop_bridge_t **bridges; ///< Array of registered bridges
  size_t bridge_count;             ///< Number of registered bridges
  size_t bridge_capacity;          ///< Bridge array capacity

  polycall_core_context_t *core_ctx; ///< Core LibPolyCall context

  /* Dynamic loading support */
  char *bridge_search_paths[16]; ///< Bridge library search paths
  size_t search_path_count;      ///< Number of search paths

  /* Configuration */
  bool auto_discovery_enabled; ///< Automatic bridge discovery
  bool strict_security_mode;   ///< Enhanced security validation
} dop_bridge_registry_t;

/* Global bridge registry instance */
static dop_bridge_registry_t *g_bridge_registry = NULL;

/* ====================================================================
 * Internal Function Declarations
 * ==================================================================== */

static polycall_dop_error_t
dop_bridge_registry_initialize_internal(polycall_core_context_t *core_ctx);

static polycall_dop_error_t
dop_bridge_registry_discover_bridges(dop_bridge_registry_t *registry);

static polycall_dop_error_t
dop_bridge_registry_load_bridge_library(dop_bridge_registry_t *registry,
                                        const char *library_path,
                                        polycall_dop_bridge_t **bridge);

static polycall_dop_error_t
dop_bridge_registry_validate_bridge(const polycall_dop_bridge_t *bridge);

static void
dop_bridge_registry_add_default_search_paths(dop_bridge_registry_t *registry);

static polycall_dop_bridge_t *
dop_bridge_registry_find_bridge_internal(dop_bridge_registry_t *registry,
                                         polycall_dop_language_t language);

/* ====================================================================
 * Bridge Registration API Implementation
 * ==================================================================== */

polycall_dop_error_t
polycall_dop_bridge_register(polycall_dop_adapter_context_t *adapter_ctx,
                             polycall_dop_language_t language,
                             polycall_dop_bridge_t *bridge) {
  if (!adapter_ctx || !bridge) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Initialize global registry if not already done
  if (!g_bridge_registry) {
    polycall_dop_error_t init_result =
        dop_bridge_registry_initialize_internal(adapter_ctx->core_ctx);
    if (init_result != POLYCALL_DOP_SUCCESS) {
      return init_result;
    }
  }

  dop_bridge_registry_t *registry = g_bridge_registry;

  // Validate bridge implementation
  polycall_dop_error_t validation_result =
      dop_bridge_registry_validate_bridge(bridge);
  if (validation_result != POLYCALL_DOP_SUCCESS) {
    return validation_result;
  }

  // Check if bridge for this language already exists
  polycall_dop_bridge_t *existing_bridge =
      dop_bridge_registry_find_bridge_internal(registry, language);
  if (existing_bridge) {
    return POLYCALL_DOP_ERROR_INVALID_STATE; // Bridge already registered
  }

  // Expand bridge array if needed
  if (registry->bridge_count >= registry->bridge_capacity) {
    size_t new_capacity =
        registry->bridge_capacity == 0 ? 8 : registry->bridge_capacity * 2;
    polycall_dop_bridge_t **new_bridges =
        (polycall_dop_bridge_t **)polycall_memory_allocate(
            registry->core_ctx, sizeof(polycall_dop_bridge_t *) * new_capacity);
    if (!new_bridges) {
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }

    if (registry->bridges) {
      memcpy(new_bridges, registry->bridges,
             sizeof(polycall_dop_bridge_t *) * registry->bridge_count);
      polycall_memory_free(registry->core_ctx, registry->bridges);
    }

    registry->bridges = new_bridges;
    registry->bridge_capacity = new_capacity;
  }

  // Initialize bridge
  if (bridge->initialize) {
    polycall_dop_error_t init_result = bridge->initialize(bridge, NULL);
    if (init_result != POLYCALL_DOP_SUCCESS) {
      return init_result;
    }
  }

  // Set bridge properties
  bridge->language = language;
  bridge->is_initialized = true;
  bridge->components_created = 0;
  bridge->total_invocations = 0;
  bridge->total_execution_time_ns = 0;
  bridge->errors_encountered = 0;

  // Register bridge
  registry->bridges[registry->bridge_count] = bridge;
  registry->bridge_count++;

  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_bridge_get(polycall_dop_adapter_context_t *adapter_ctx,
                        polycall_dop_language_t language,
                        polycall_dop_bridge_t **bridge) {
  if (!adapter_ctx || !bridge) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  *bridge = NULL;

  // Initialize registry if not already done
  if (!g_bridge_registry) {
    polycall_dop_error_t init_result =
        dop_bridge_registry_initialize_internal(adapter_ctx->core_ctx);
    if (init_result != POLYCALL_DOP_SUCCESS) {
      return init_result;
    }
  }

  dop_bridge_registry_t *registry = g_bridge_registry;

  // Find bridge for specified language
  polycall_dop_bridge_t *found_bridge =
      dop_bridge_registry_find_bridge_internal(registry, language);
  if (!found_bridge) {
    // Try auto-discovery if enabled
    if (registry->auto_discovery_enabled) {
      polycall_dop_error_t discovery_result =
          dop_bridge_registry_discover_bridges(registry);
      if (discovery_result == POLYCALL_DOP_SUCCESS) {
        found_bridge =
            dop_bridge_registry_find_bridge_internal(registry, language);
      }
    }
  }

  if (!found_bridge) {
    return POLYCALL_DOP_ERROR_BRIDGE_UNAVAILABLE;
  }

  *bridge = found_bridge;
  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_bridge_unregister(polycall_dop_adapter_context_t *adapter_ctx,
                               polycall_dop_language_t language) {
  if (!adapter_ctx) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!g_bridge_registry) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  dop_bridge_registry_t *registry = g_bridge_registry;

  // Find bridge index
  size_t bridge_index = SIZE_MAX;
  for (size_t i = 0; i < registry->bridge_count; i++) {
    if (registry->bridges[i]->language == language) {
      bridge_index = i;
      break;
    }
  }

  if (bridge_index == SIZE_MAX) {
    return POLYCALL_DOP_ERROR_BRIDGE_UNAVAILABLE;
  }

  polycall_dop_bridge_t *bridge = registry->bridges[bridge_index];

  // Cleanup bridge
  if (bridge->cleanup) {
    bridge->cleanup(bridge);
  }

  // Close dynamic library handle if present
  if (bridge->bridge_handle) {
    dlclose(bridge->bridge_handle);
  }

  // Free bridge memory
  polycall_memory_free(registry->core_ctx, bridge);

  // Remove from registry
  for (size_t i = bridge_index; i < registry->bridge_count - 1; i++) {
    registry->bridges[i] = registry->bridges[i + 1];
  }
  registry->bridge_count--;

  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_bridge_list_available(polycall_dop_adapter_context_t *adapter_ctx,
                                   polycall_dop_language_t *languages,
                                   size_t *language_count,
                                   size_t max_languages) {
  if (!adapter_ctx || !language_count) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!g_bridge_registry) {
    *language_count = 0;
    return POLYCALL_DOP_SUCCESS;
  }

  dop_bridge_registry_t *registry = g_bridge_registry;

  size_t count = 0;
  for (size_t i = 0; i < registry->bridge_count && count < max_languages; i++) {
    if (languages) {
      languages[count] = registry->bridges[i]->language;
    }
    count++;
  }

  *language_count = count;
  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Built-in Bridge Implementations
 * ==================================================================== */

/**
 * @brief C Bridge Implementation (Native)
 */
static polycall_dop_error_t
c_bridge_initialize(polycall_dop_bridge_t *bridge,
                    const polycall_dop_component_config_t *config) {
  // C components don't need special initialization
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
c_bridge_create_component(polycall_dop_bridge_t *bridge,
                          const polycall_dop_component_config_t *config,
                          polycall_dop_component_t *component) {
  // C components are handled natively
  bridge->components_created++;
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
c_bridge_invoke_method(polycall_dop_bridge_t *bridge,
                       polycall_dop_component_t *component,
                       const polycall_dop_invocation_t *invocation,
                       polycall_dop_result_t *result) {
  // C method invocation through function pointers
  bridge->total_invocations++;

  // This would call the actual C function
  // For now, return success as placeholder
  result->error_code = POLYCALL_DOP_SUCCESS;
  result->return_value = polycall_dop_value_create_null();

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
c_bridge_destroy_component(polycall_dop_bridge_t *bridge,
                           polycall_dop_component_t *component) {
  // C component cleanup
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t c_bridge_cleanup(polycall_dop_bridge_t *bridge) {
  // C bridge cleanup
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
c_bridge_convert_to_native(polycall_dop_bridge_t *bridge,
                           const polycall_dop_value_t *dop_value,
                           void **native_value) {
  // C values are already native
  *native_value = (void *)dop_value;
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
c_bridge_convert_from_native(polycall_dop_bridge_t *bridge,
                             const void *native_value,
                             polycall_dop_value_t *dop_value) {
  // C values are already native
  *dop_value = *(const polycall_dop_value_t *)native_value;
  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t polycall_dop_bridge_register_builtin_bridges(
    polycall_dop_adapter_context_t *adapter_ctx) {
  if (!adapter_ctx) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Register C bridge
  polycall_dop_bridge_t *c_bridge =
      (polycall_dop_bridge_t *)polycall_memory_allocate(
          adapter_ctx->core_ctx, sizeof(polycall_dop_bridge_t));
  if (!c_bridge) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  memset(c_bridge, 0, sizeof(polycall_dop_bridge_t));
  c_bridge->bridge_name = "c_bridge";
  c_bridge->version = "1.0.0";
  c_bridge->initialize = c_bridge_initialize;
  c_bridge->create_component = c_bridge_create_component;
  c_bridge->invoke_method = c_bridge_invoke_method;
  c_bridge->destroy_component = c_bridge_destroy_component;
  c_bridge->cleanup = c_bridge_cleanup;
  c_bridge->convert_to_native = c_bridge_convert_to_native;
  c_bridge->convert_from_native = c_bridge_convert_from_native;

  polycall_dop_error_t c_result = polycall_dop_bridge_register(
      adapter_ctx, POLYCALL_DOP_LANGUAGE_C, c_bridge);
  if (c_result != POLYCALL_DOP_SUCCESS) {
    polycall_memory_free(adapter_ctx->core_ctx, c_bridge);
    return c_result;
  }

  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Internal Function Implementations
 * ==================================================================== */

static polycall_dop_error_t
dop_bridge_registry_initialize_internal(polycall_core_context_t *core_ctx) {
  if (g_bridge_registry) {
    return POLYCALL_DOP_SUCCESS; // Already initialized
  }

  g_bridge_registry = (dop_bridge_registry_t *)polycall_memory_allocate(
      core_ctx, sizeof(dop_bridge_registry_t));
  if (!g_bridge_registry) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  memset(g_bridge_registry, 0, sizeof(dop_bridge_registry_t));
  g_bridge_registry->core_ctx = core_ctx;
  g_bridge_registry->auto_discovery_enabled = true;
  g_bridge_registry->strict_security_mode = true;

  // Add default search paths
  dop_bridge_registry_add_default_search_paths(g_bridge_registry);

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_bridge_registry_discover_bridges(dop_bridge_registry_t *registry) {
  // Automatic bridge discovery implementation
  // This would scan the search paths for bridge libraries

  const char *bridge_files[] = {"libpolycall_dop_bridge_js.so",
                                "libpolycall_dop_bridge_python.so",
                                "libpolycall_dop_bridge_jvm.so"};

  for (size_t i = 0; i < registry->search_path_count; i++) {
    for (size_t j = 0; j < sizeof(bridge_files) / sizeof(bridge_files[0]);
         j++) {
      char full_path[512];
      snprintf(full_path, sizeof(full_path), "%s/%s",
               registry->bridge_search_paths[i], bridge_files[j]);

      // Check if file exists and try to load
      if (access(full_path, F_OK) == 0) {
        polycall_dop_bridge_t *bridge = NULL;
        polycall_dop_error_t load_result =
            dop_bridge_registry_load_bridge_library(registry, full_path,
                                                    &bridge);
        if (load_result == POLYCALL_DOP_SUCCESS && bridge) {
          // Bridge loaded successfully
          registry->bridges[registry->bridge_count] = bridge;
          registry->bridge_count++;
        }
      }
    }
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_bridge_registry_load_bridge_library(dop_bridge_registry_t *registry,
                                        const char *library_path,
                                        polycall_dop_bridge_t **bridge) {
  if (!registry || !library_path || !bridge) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Load dynamic library
  void *handle = dlopen(library_path, RTLD_LAZY);
  if (!handle) {
    return POLYCALL_DOP_ERROR_BRIDGE_UNAVAILABLE;
  }

  // Get bridge factory function
  typedef polycall_dop_bridge_t *(*bridge_factory_t)(void);
  bridge_factory_t factory =
      (bridge_factory_t)dlsym(handle, "polycall_dop_bridge_create");

  if (!factory) {
    dlclose(handle);
    return POLYCALL_DOP_ERROR_BRIDGE_UNAVAILABLE;
  }

  // Create bridge instance
  polycall_dop_bridge_t *new_bridge = factory();
  if (!new_bridge) {
    dlclose(handle);
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Validate bridge
  polycall_dop_error_t validation_result =
      dop_bridge_registry_validate_bridge(new_bridge);
  if (validation_result != POLYCALL_DOP_SUCCESS) {
    dlclose(handle);
    return validation_result;
  }

  new_bridge->bridge_handle = handle;
  *bridge = new_bridge;

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_bridge_registry_validate_bridge(const polycall_dop_bridge_t *bridge) {
  if (!bridge) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Validate required function pointers
  if (!bridge->create_component || !bridge->invoke_method ||
      !bridge->destroy_component) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  // Validate bridge name and version
  if (!bridge->bridge_name || !bridge->version) {
    return POLYCALL_DOP_ERROR_CONFIGURATION_INVALID;
  }

  return POLYCALL_DOP_SUCCESS;
}

static void
dop_bridge_registry_add_default_search_paths(dop_bridge_registry_t *registry) {
  const char *default_paths[] = {
      "./lib", "./build/lib", "/usr/local/lib/polycall", "/usr/lib/polycall"};

  size_t path_count = sizeof(default_paths) / sizeof(default_paths[0]);
  for (size_t i = 0; i < path_count && registry->search_path_count < 16; i++) {
    size_t path_len = strlen(default_paths[i]);
    registry->bridge_search_paths[registry->search_path_count] =
        (char *)polycall_memory_allocate(registry->core_ctx, path_len + 1);
    if (registry->bridge_search_paths[registry->search_path_count]) {
      strcpy(registry->bridge_search_paths[registry->search_path_count],
             default_paths[i]);
      registry->search_path_count++;
    }
  }

  // Add environment variable path if set
  const char *env_path = getenv("POLYCALL_BRIDGE_PATH");
  if (env_path && registry->search_path_count < 16) {
    size_t path_len = strlen(env_path);
    registry->bridge_search_paths[registry->search_path_count] =
        (char *)polycall_memory_allocate(registry->core_ctx, path_len + 1);
    if (registry->bridge_search_paths[registry->search_path_count]) {
      strcpy(registry->bridge_search_paths[registry->search_path_count],
             env_path);
      registry->search_path_count++;
    }
  }
}

static polycall_dop_bridge_t *
dop_bridge_registry_find_bridge_internal(dop_bridge_registry_t *registry,
                                         polycall_dop_language_t language) {
  for (size_t i = 0; i < registry->bridge_count; i++) {
    if (registry->bridges[i]->language == language) {
      return registry->bridges[i];
    }
  }
  return NULL;
}

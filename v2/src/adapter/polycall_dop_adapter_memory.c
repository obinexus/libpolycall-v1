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
 * @file polycall_dop_adapter_memory.c
 * @brief DOP Adapter Memory Management Implementation
 *
 * LibPolyCall DOP Adapter Framework - Memory Bridge Implementation
 * OBINexus Computing - Aegis Project Technical Infrastructure
 *
 * Implements secure memory management with strict isolation boundaries,
 * reference counting, and Zero Trust memory sharing policies.
 * Essential for banking app security (ads service vs payment component
 * isolation).
 *
 * @version 1.0.0
 * @date 2025-06-09
 */

#include "polycall/core/dop/polycall_dop_adapter.h"
#include "polycall/core/polycall_core.h"
#include "polycall/core/polycall_memory.h"
#include "polycall/core/polycall_security.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

/* ====================================================================
 * Memory Manager Internal Structure
 * ==================================================================== */

struct polycall_dop_memory_manager {
  polycall_core_context_t *core_ctx; ///< Core LibPolyCall context

  /* Memory Region Registry */
  polycall_dop_memory_region_t *regions; ///< All allocated regions
  size_t region_count;                   ///< Number of active regions
  size_t region_capacity;                ///< Region array capacity

  /* Memory Pools */
  struct {
    void *base_address; ///< Pool base address
    size_t total_size;  ///< Total pool size
    size_t used_size;   ///< Currently used size
    bool is_protected;  ///< Memory protection enabled
  } memory_pools[8];    ///< Pre-allocated memory pools
  size_t pool_count;    ///< Number of active pools

  /* Isolation Boundaries */
  struct {
    const char *component_id;                    ///< Component owning boundary
    void *start_address;                         ///< Boundary start
    void *end_address;                           ///< Boundary end
    polycall_dop_permission_flags_t permissions; ///< Access permissions
  } isolation_boundaries[POLYCALL_DOP_MAX_MEMORY_REGIONS];
  size_t boundary_count; ///< Number of boundaries

  /* Memory Guard Configuration */
  bool guard_pages_enabled;       ///< Guard page protection
  bool canary_protection_enabled; ///< Canary value protection
  bool zero_on_free_enabled;      ///< Zero memory on free
  size_t guard_page_size;         ///< Guard page size

  /* Statistics */
  uint64_t total_allocated;     ///< Total bytes allocated
  uint64_t total_freed;         ///< Total bytes freed
  uint64_t peak_usage;          ///< Peak memory usage
  uint64_t allocation_count;    ///< Number of allocations
  uint64_t free_count;          ///< Number of frees
  uint64_t boundary_violations; ///< Boundary violation count
};

/* ====================================================================
 * Internal Function Declarations
 * ==================================================================== */

static polycall_dop_error_t
dop_memory_initialize_pools(polycall_dop_memory_manager_t *memory_mgr);

static polycall_dop_error_t
dop_memory_allocate_region(polycall_dop_memory_manager_t *memory_mgr,
                           size_t size,
                           polycall_dop_permission_flags_t permissions,
                           polycall_dop_memory_region_t **region);

static polycall_dop_error_t dop_memory_validate_access(
    polycall_dop_memory_manager_t *memory_mgr, const char *component_id,
    void *address, size_t size,
    polycall_dop_permission_flags_t requested_permissions);

static polycall_dop_error_t
dop_memory_setup_guard_pages(polycall_dop_memory_manager_t *memory_mgr,
                             polycall_dop_memory_region_t *region);

static polycall_dop_error_t
dop_memory_remove_guard_pages(polycall_dop_memory_manager_t *memory_mgr,
                              polycall_dop_memory_region_t *region);

static polycall_dop_error_t
dop_memory_register_boundary(polycall_dop_memory_manager_t *memory_mgr,
                             const char *component_id, void *start_address,
                             void *end_address,
                             polycall_dop_permission_flags_t permissions);

static bool dop_memory_address_in_boundary(
    const polycall_dop_memory_manager_t *memory_mgr, const char *component_id,
    void *address, polycall_dop_permission_flags_t requested_permissions);

static void dop_memory_zero_region(void *address, size_t size);

static size_t dop_memory_get_page_size(void);

/* ====================================================================
 * Memory Management API Implementation
 * ==================================================================== */

polycall_dop_error_t
polycall_dop_memory_allocate(polycall_dop_adapter_context_t *adapter_ctx,
                             polycall_dop_component_t *component, size_t size,
                             polycall_dop_permission_flags_t permissions,
                             polycall_dop_memory_region_t **region) {
  if (!adapter_ctx || !component || !region || size == 0) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!adapter_ctx->memory) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  polycall_dop_memory_manager_t *memory_mgr = adapter_ctx->memory;
  polycall_dop_error_t result = POLYCALL_DOP_SUCCESS;

  // Validate component has permission to allocate memory
  if (!(component->security_policy.allowed_permissions &
        POLYCALL_DOP_PERMISSION_MEMORY_WRITE)) {
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Check if allocation would exceed component memory limits
  size_t total_after_allocation = component->total_memory_allocated + size;
  if (total_after_allocation > component->security_policy.max_memory_usage) {
    return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
  }

  // Allocate memory region
  polycall_dop_memory_region_t *new_region = NULL;
  result =
      dop_memory_allocate_region(memory_mgr, size, permissions, &new_region);
  if (result != POLYCALL_DOP_SUCCESS) {
    return result;
  }

  // Set region ownership
  new_region->owner_component_id = component->component_id;
  new_region->reference_count = 1;
  new_region->is_shared = false;
  new_region->permissions = permissions;

  // Setup guard pages if enabled
  if (memory_mgr->guard_pages_enabled &&
      component->security_policy.isolation_level >=
          POLYCALL_DOP_ISOLATION_STRICT) {
    result = dop_memory_setup_guard_pages(memory_mgr, new_region);
    if (result != POLYCALL_DOP_SUCCESS) {
      // Cleanup allocated region
      polycall_memory_free(adapter_ctx->core_ctx, new_region->base_address);
      polycall_memory_free(adapter_ctx->core_ctx, new_region);
      return result;
    }
  }

  // Register isolation boundary
  result = dop_memory_register_boundary(
      memory_mgr, component->component_id, new_region->base_address,
      (void *)((char *)new_region->base_address + new_region->size),
      permissions);
  if (result != POLYCALL_DOP_SUCCESS) {
    // Cleanup guard pages and region
    if (memory_mgr->guard_pages_enabled) {
      dop_memory_remove_guard_pages(memory_mgr, new_region);
    }
    polycall_memory_free(adapter_ctx->core_ctx, new_region->base_address);
    polycall_memory_free(adapter_ctx->core_ctx, new_region);
    return result;
  }

  // Add region to component's region list
  if (component->region_count == 0) {
    component->memory_regions =
        (polycall_dop_memory_region_t *)polycall_memory_allocate(
            adapter_ctx->core_ctx,
            sizeof(polycall_dop_memory_region_t) * 16 // Initial capacity
        );
    if (!component->memory_regions) {
      // Cleanup everything
      dop_memory_remove_guard_pages(memory_mgr, new_region);
      polycall_memory_free(adapter_ctx->core_ctx, new_region->base_address);
      polycall_memory_free(adapter_ctx->core_ctx, new_region);
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }
  }

  // Copy region info to component's list
  memcpy(&component->memory_regions[component->region_count], new_region,
         sizeof(polycall_dop_memory_region_t));
  component->region_count++;

  // Update component statistics
  component->total_memory_allocated += size;

  // Update memory manager statistics
  memory_mgr->total_allocated += size;
  memory_mgr->allocation_count++;
  if (memory_mgr->total_allocated - memory_mgr->total_freed >
      memory_mgr->peak_usage) {
    memory_mgr->peak_usage =
        memory_mgr->total_allocated - memory_mgr->total_freed;
  }

  // Log allocation event
  polycall_dop_audit_event_t audit_event = {
      .event_type = POLYCALL_DOP_AUDIT_MEMORY_ALLOCATED,
      .timestamp_ns = dop_security_get_monotonic_time_ns(),
      .component_id = component->component_id,
      .method_name = "memory_allocate",
      .error_code = POLYCALL_DOP_SUCCESS,
      .details = "Memory region allocated successfully",
      .context_data = new_region,
      .context_size = sizeof(polycall_dop_memory_region_t)};
  polycall_dop_audit_log(adapter_ctx, &audit_event);

  *region = new_region;
  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_memory_free(polycall_dop_adapter_context_t *adapter_ctx,
                         polycall_dop_component_t *component,
                         polycall_dop_memory_region_t *region) {
  if (!adapter_ctx || !component || !region) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!adapter_ctx->memory) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  polycall_dop_memory_manager_t *memory_mgr = adapter_ctx->memory;

  // Validate component owns this region
  if (strcmp(region->owner_component_id, component->component_id) != 0) {
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Check reference count
  if (region->reference_count > 1) {
    // Decrement reference count but don't free yet
    region->reference_count--;
    return POLYCALL_DOP_SUCCESS;
  }

  // Zero memory if configured for security
  if (memory_mgr->zero_on_free_enabled) {
    dop_memory_zero_region(region->base_address, region->size);
  }

  // Remove guard pages if present
  if (memory_mgr->guard_pages_enabled) {
    dop_memory_remove_guard_pages(memory_mgr, region);
  }

  // Free the actual memory
  polycall_memory_free(adapter_ctx->core_ctx, region->base_address);

  // Remove from component's region list
  for (size_t i = 0; i < component->region_count; i++) {
    if (&component->memory_regions[i] == region) {
      // Shift remaining regions down
      for (size_t j = i; j < component->region_count - 1; j++) {
        memcpy(&component->memory_regions[j], &component->memory_regions[j + 1],
               sizeof(polycall_dop_memory_region_t));
      }
      component->region_count--;
      break;
    }
  }

  // Update statistics
  memory_mgr->total_freed += region->size;
  memory_mgr->free_count++;
  component->total_memory_allocated -= region->size;

  // Log free event
  polycall_dop_audit_event_t audit_event = {
      .event_type = POLYCALL_DOP_AUDIT_MEMORY_FREED,
      .timestamp_ns = dop_security_get_monotonic_time_ns(),
      .component_id = component->component_id,
      .method_name = "memory_free",
      .error_code = POLYCALL_DOP_SUCCESS,
      .details = "Memory region freed successfully",
      .context_data = NULL,
      .context_size = 0};
  polycall_dop_audit_log(adapter_ctx, &audit_event);

  return POLYCALL_DOP_SUCCESS;
}

polycall_dop_error_t
polycall_dop_memory_share(polycall_dop_adapter_context_t *adapter_ctx,
                          polycall_dop_component_t *source_component,
                          polycall_dop_component_t *target_component,
                          polycall_dop_memory_region_t *region,
                          polycall_dop_permission_flags_t permissions) {
  if (!adapter_ctx || !source_component || !target_component || !region) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (!adapter_ctx->memory) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  polycall_dop_memory_manager_t *memory_mgr = adapter_ctx->memory;

  // Validate source component owns the region
  if (strcmp(region->owner_component_id, source_component->component_id) != 0) {
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Validate source component has permission to share memory
  if (!(source_component->security_policy.allowed_permissions &
        POLYCALL_DOP_PERMISSION_MEMORY_READ)) {
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Validate target component has permission to access shared memory
  if (!(target_component->security_policy.allowed_permissions & permissions)) {
    return POLYCALL_DOP_ERROR_PERMISSION_DENIED;
  }

  // Check isolation levels allow sharing
  polycall_dop_isolation_level_t min_isolation =
      (source_component->security_policy.isolation_level <
       target_component->security_policy.isolation_level)
          ? source_component->security_policy.isolation_level
          : target_component->security_policy.isolation_level;

  if (min_isolation >= POLYCALL_DOP_ISOLATION_PARANOID) {
    return POLYCALL_DOP_ERROR_ISOLATION_BREACH; // Paranoid isolation doesn't
                                                // allow sharing
  }

  // Check if region is already shared
  if (region->is_shared) {
    // Increment reference count
    region->reference_count++;
  } else {
    // Mark as shared and increment reference count
    region->is_shared = true;
    region->reference_count++;
  }

  // Register boundary for target component
  polycall_dop_error_t result = dop_memory_register_boundary(
      memory_mgr, target_component->component_id, region->base_address,
      (void *)((char *)region->base_address + region->size), permissions);

  if (result != POLYCALL_DOP_SUCCESS) {
    // Rollback reference count
    region->reference_count--;
    if (region->reference_count <= 1) {
      region->is_shared = false;
    }
    return result;
  }

  // Add region to target component's region list (as a shared reference)
  if (target_component->region_count == 0) {
    target_component->memory_regions =
        (polycall_dop_memory_region_t *)polycall_memory_allocate(
            adapter_ctx->core_ctx, sizeof(polycall_dop_memory_region_t) * 16);
    if (!target_component->memory_regions) {
      // Rollback
      region->reference_count--;
      if (region->reference_count <= 1) {
        region->is_shared = false;
      }
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }
  }

  // Copy region with updated permissions
  polycall_dop_memory_region_t shared_region = *region;
  shared_region.permissions = permissions;
  memcpy(&target_component->memory_regions[target_component->region_count],
         &shared_region, sizeof(polycall_dop_memory_region_t));
  target_component->region_count++;

  // Log sharing event
  polycall_dop_audit_event_t audit_event = {
      .event_type = POLYCALL_DOP_AUDIT_MEMORY_ALLOCATED, // Using allocation
                                                         // event for sharing
      .timestamp_ns = dop_security_get_monotonic_time_ns(),
      .component_id = target_component->component_id,
      .method_name = "memory_share",
      .error_code = POLYCALL_DOP_SUCCESS,
      .details = "Memory region shared successfully",
      .context_data = region,
      .context_size = sizeof(polycall_dop_memory_region_t)};
  polycall_dop_audit_log(adapter_ctx, &audit_event);

  return POLYCALL_DOP_SUCCESS;
}

/* ====================================================================
 * Internal Helper Function Implementations
 * ==================================================================== */

static polycall_dop_error_t
dop_memory_initialize_pools(polycall_dop_memory_manager_t *memory_mgr) {
  if (!memory_mgr) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Initialize memory pools for different allocation sizes
  size_t pool_sizes[] = {
      4096,     // 4KB pool
      16384,    // 16KB pool
      65536,    // 64KB pool
      262144,   // 256KB pool
      1048576,  // 1MB pool
      4194304,  // 4MB pool
      16777216, // 16MB pool
      67108864  // 64MB pool
  };

  memory_mgr->pool_count = sizeof(pool_sizes) / sizeof(pool_sizes[0]);

  for (size_t i = 0; i < memory_mgr->pool_count; i++) {
    void *pool_memory =
        polycall_memory_allocate(memory_mgr->core_ctx, pool_sizes[i]);
    if (!pool_memory) {
      // Cleanup previously allocated pools
      for (size_t j = 0; j < i; j++) {
        polycall_memory_free(memory_mgr->core_ctx,
                             memory_mgr->memory_pools[j].base_address);
      }
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }

    memory_mgr->memory_pools[i] =
        (typeof(memory_mgr->memory_pools[0])){.base_address = pool_memory,
                                              .total_size = pool_sizes[i],
                                              .used_size = 0,
                                              .is_protected = false};
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_memory_allocate_region(polycall_dop_memory_manager_t *memory_mgr,
                           size_t size,
                           polycall_dop_permission_flags_t permissions,
                           polycall_dop_memory_region_t **region) {
  if (!memory_mgr || !region || size == 0) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Allocate region structure
  polycall_dop_memory_region_t *new_region =
      (polycall_dop_memory_region_t *)polycall_memory_allocate(
          memory_mgr->core_ctx, sizeof(polycall_dop_memory_region_t));
  if (!new_region) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Allocate actual memory
  void *memory = polycall_memory_allocate(memory_mgr->core_ctx, size);
  if (!memory) {
    polycall_memory_free(memory_mgr->core_ctx, new_region);
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Initialize region
  memset(new_region, 0, sizeof(polycall_dop_memory_region_t));
  new_region->base_address = memory;
  new_region->size = size;
  new_region->permissions = permissions;
  new_region->reference_count = 0; // Will be set by caller
  new_region->is_shared = false;

  // Add to global region registry
  if (memory_mgr->region_count >= memory_mgr->region_capacity) {
    // Expand region registry
    size_t new_capacity =
        memory_mgr->region_capacity == 0 ? 64 : memory_mgr->region_capacity * 2;
    polycall_dop_memory_region_t *new_regions =
        (polycall_dop_memory_region_t *)polycall_memory_allocate(
            memory_mgr->core_ctx,
            sizeof(polycall_dop_memory_region_t) * new_capacity);
    if (!new_regions) {
      polycall_memory_free(memory_mgr->core_ctx, memory);
      polycall_memory_free(memory_mgr->core_ctx, new_region);
      return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
    }

    if (memory_mgr->regions) {
      memcpy(new_regions, memory_mgr->regions,
             sizeof(polycall_dop_memory_region_t) * memory_mgr->region_count);
      polycall_memory_free(memory_mgr->core_ctx, memory_mgr->regions);
    }

    memory_mgr->regions = new_regions;
    memory_mgr->region_capacity = new_capacity;
  }

  // Add to registry
  memcpy(&memory_mgr->regions[memory_mgr->region_count], new_region,
         sizeof(polycall_dop_memory_region_t));
  memory_mgr->region_count++;

  *region = new_region;
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t dop_memory_validate_access(
    polycall_dop_memory_manager_t *memory_mgr, const char *component_id,
    void *address, size_t size,
    polycall_dop_permission_flags_t requested_permissions) {
  if (!memory_mgr || !component_id || !address) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Check if address is within component's boundaries
  if (!dop_memory_address_in_boundary(memory_mgr, component_id, address,
                                      requested_permissions)) {
    memory_mgr->boundary_violations++;
    return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
  }

  // Validate access doesn't exceed region bounds
  void *end_address = (void *)((char *)address + size);
  if (!dop_memory_address_in_boundary(memory_mgr, component_id, end_address,
                                      requested_permissions)) {
    memory_mgr->boundary_violations++;
    return POLYCALL_DOP_ERROR_ISOLATION_BREACH;
  }

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_memory_setup_guard_pages(polycall_dop_memory_manager_t *memory_mgr,
                             polycall_dop_memory_region_t *region) {
  if (!memory_mgr || !region) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Get system page size
  size_t page_size = dop_memory_get_page_size();
  if (page_size == 0) {
    return POLYCALL_DOP_ERROR_INVALID_STATE;
  }

  // Setup guard pages before and after the region
  void *guard_before = (void *)((char *)region->base_address - page_size);
  void *guard_after = (void *)((char *)region->base_address + region->size);

  // Make guard pages non-accessible (this would use mprotect in a real
  // implementation) For now, this is a placeholder
  memory_mgr->guard_page_size = page_size;

  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_memory_remove_guard_pages(polycall_dop_memory_manager_t *memory_mgr,
                              polycall_dop_memory_region_t *region) {
  if (!memory_mgr || !region) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  // Remove guard page protection (placeholder implementation)
  return POLYCALL_DOP_SUCCESS;
}

static polycall_dop_error_t
dop_memory_register_boundary(polycall_dop_memory_manager_t *memory_mgr,
                             const char *component_id, void *start_address,
                             void *end_address,
                             polycall_dop_permission_flags_t permissions) {
  if (!memory_mgr || !component_id || !start_address || !end_address) {
    return POLYCALL_DOP_ERROR_INVALID_PARAMETER;
  }

  if (memory_mgr->boundary_count >= POLYCALL_DOP_MAX_MEMORY_REGIONS) {
    return POLYCALL_DOP_ERROR_MEMORY_ALLOCATION;
  }

  // Register the boundary
  memory_mgr->isolation_boundaries[memory_mgr->boundary_count] =
      (typeof(memory_mgr->isolation_boundaries[0])){
          .component_id = component_id,
          .start_address = start_address,
          .end_address = end_address,
          .permissions = permissions};
  memory_mgr->boundary_count++;

  return POLYCALL_DOP_SUCCESS;
}

static bool dop_memory_address_in_boundary(
    const polycall_dop_memory_manager_t *memory_mgr, const char *component_id,
    void *address, polycall_dop_permission_flags_t requested_permissions) {
  if (!memory_mgr || !component_id || !address) {
    return false;
  }

  // Check all boundaries for this component
  for (size_t i = 0; i < memory_mgr->boundary_count; i++) {
    if (strcmp(memory_mgr->isolation_boundaries[i].component_id,
               component_id) == 0) {
      void *start = memory_mgr->isolation_boundaries[i].start_address;
      void *end = memory_mgr->isolation_boundaries[i].end_address;
      polycall_dop_permission_flags_t boundary_permissions =
          memory_mgr->isolation_boundaries[i].permissions;

      // Check if address is within boundary
      if (address >= start && address < end) {
        // Check if permissions are sufficient
        if ((boundary_permissions & requested_permissions) ==
            requested_permissions) {
          return true;
        }
      }
    }
  }

  return false;
}

static void dop_memory_zero_region(void *address, size_t size) {
  if (address && size > 0) {
    memset(address, 0, size);
  }
}

static size_t dop_memory_get_page_size(void) {
  long page_size = sysconf(_SC_PAGESIZE);
  return (page_size > 0) ? (size_t)page_size : 4096; // Fallback to 4KB
}

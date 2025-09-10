# Hotwiring Adapter Integration Specification

This document outlines the proposed adapter layer used to bridge LibPolyCall's C runtime with language specific bindings. It consolidates the architecture description from the *Hotwiring Adapter Integration Specification for LibPolyCall*.

## Base C Adapter Interface

The adapter layer defines a common `adapter_base_t` structure with a vtable for lifecycle functions. The interface maintains a `topology_manager_t` pointer, an atomic reference count and thread safety primitives. Key API functions include:

- `adapter_base_init`
- `adapter_base_acquire`
- `adapter_base_release`
- `adapter_execute_transition`

These functions provide safe initialization and transition execution for all language bindings.

## Language-Specific Implementations

Implementations for Python (CFFI), Go (CGO) and Node.js (N-API) are provided as examples. Each adapter embeds `adapter_base_t` and implements `enter_layer` to coordinate with its respective runtime. The design ensures that entering a new layer validates the transition using `adapter_execute_transition` before invoking language specific callbacks or event queues.

```c
typedef struct python_adapter {
    adapter_base_t base;
    PyObject* callback_dict;
    PyGILState_STATE gil_state;
} python_adapter_t;
```

The Go adapter ensures transitions occur on the owning OS thread to satisfy CGO restrictions, while the Node.js adapter schedules asynchronous notifications to the event loop using `uv_async_t`.

## Unified Adapter Registry

A global `adapter_registry_t` maintains registered adapters for each topology layer and coordinates transitions between them. The `adapter_orchestrate_transition` function demonstrates a thread-safe approach to exiting one layer and entering another using the registry.

## Testing Considerations

Unit tests should verify thread safety, reference counting and correct transition validation across adapters. Integration tests focus on cross-language transitions to guarantee consistent behaviour.


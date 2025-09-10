# Hotwiring Adapter Integration Specification

This document summarizes the core adapter interface design and multi-language integration strategy for LibPolyCall. The goal is to provide a unified, thread-safe adapter layer that supports Python, Go, and Node.js bindings while maintaining consistent lifecycle management across the C runtime.

## Core Adapter Interface

The base adapter structure defines function pointers for initialization, layer transitions, trace emission, and cleanup. Each adapter maintains a reference to the global topology manager and uses an atomic reference counter for memory safety.

Key fields:
- `vtable` – language-specific implementations of the adapter operations
- `manager` – pointer to the shared topology manager
- `ref_count` – atomic reference counter for lifecycle management
- `mutex` – mutex protecting adapter state
- `language_specific_data` – pointer reserved for binding data
- `adapter_layer_id` – numeric identifier for the adapter

Thread-safe transitions are performed through `adapter_execute_transition`, which validates layer changes and updates thread context.

## Language-Specific Implementations

### Python CFFI Adapter
- Acquires the Python GIL before executing a transition
- Stores callbacks in a `PyDict` for dispatch
- Sets `adapter_layer_id` to the Python layer constant

### Go CGO Adapter
- Ensures transitions occur on the originating OS thread
- Notifies the Go runtime of successful transitions via CGO callbacks
- Stores a `go_handle` used by exported Go functions

### Node.js N-API Adapter
- Uses `napi_env` and `uv_async_t` for event-loop safe transitions
- Opens and closes handle scopes around calls into the Node.js runtime
- Queues asynchronous notifications on successful layer entry

## Adapter Registry

Adapters are stored in a registry indexed by layer ID. A read–write lock guards registry access so that multiple threads can look up adapters concurrently while registration occurs under a write lock.

## Transition Orchestration

The orchestrator coordinates transitions between adapters. It first exits the current layer using the source adapter and then enters the target layer using the destination adapter. Errors in either step abort the transition and propagate back to the caller.

## Testing and Performance

Unit tests exercise thread safety by spawning multiple threads that perform rapid transitions. Integration tests verify that a Python layer can safely transition into a Go layer and back again. Transition validation and trace emission are benchmarked to ensure targets of less than 100µs per transition.


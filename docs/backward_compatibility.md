# Hotwiring for Backward Compatibility

Hotwiring in LibPolyCall allows developers to replace legacy API functionality without breaking existing integrations. By routing old command paths to new implementation modules, applications can gradually adopt updated features while remaining functional.

## Approach

1. **Compatibility Layer** – Provide thin wrappers around deprecated functions. These wrappers forward requests to new modules using the Sinphasé command interface.
2. **Incremental Replacement** – Update one feature at a time. Each replacement includes cost analysis to confirm that the new code stays within acceptable complexity thresholds.
3. **Telemetry Hooks** – Maintain telemetry in both old and new code paths to verify behaviour and gather usage statistics.
4. **Isolation Triggers** – If a new module exceeds the configured cost threshold, it can be isolated into a separate component until optimised.

This strategy ensures stable evolution of the runtime while following the single-pass, hierarchical structuring pattern promoted by the Sinphasé framework.


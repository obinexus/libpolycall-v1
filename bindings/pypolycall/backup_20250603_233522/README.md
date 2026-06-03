# PyPolyCall - LibPolyCall Trial v1 Python Binding

> **Binding Location**: `bindings/pypolycall`
> **Installation**: `pip install git+https://github.com/obinexus/libpolycall-v1trial.git#subdirectory=bindings/pypolycall`


**Protocol-Compliant Adapter for polycall.exe Runtime**

## ⚠️ PROTOCOL COMPLIANCE NOTICE

This Python binding is an **ADAPTER** for the LibPolyCall Trial v1 runtime (`polycall.exe`). It does **NOT** function as a standalone server or API framework.

### Protocol Law Requirements:
- ✅ All execution must go through `polycall.exe` runtime
- ✅ Binding acts as adapter, never bypasses protocol
- ✅ User logic submitted via FFI and config declarations
- ✅ Zero-trust architecture with state machine validation
- ✅ Cryptographic seeding and continuous permission checking

## Installation

```bash
pip install -e .
```

### Development Installation

```bash
pip install -e ".[dev]"
```

## Quick Start

### 1. Start polycall.exe Runtime

First, ensure the LibPolyCall runtime is running:

```bash
# Start polycall.exe runtime (default port 8084)
polycall.exe server --port 8084
```

### 2. Connect Python Binding

```python
import asyncio
from pypolycall import PolyCallClient

async def main():
    # Create client adapter
    client = PolyCallClient(host="localhost", port=8084)
    
    # Connect to polycall.exe runtime
    await client.connect()
    
    # Authenticate (zero-trust required)
    await client.authenticate({"username": "user", "password": "pass"})
    
    # Register user logic (submitted to polycall.exe)
    @client.handler("/api/hello")
    async def hello_handler(request):
        return {"message": "Hello from PyPolyCall adapter"}
    
    # Keep adapter running
    await client.run()

if __name__ == "__main__":
    asyncio.run(main())
```

### 3. CLI Usage

```bash
# Test connection to polycall.exe
pypolycall test --host localhost --port 8084

# Start client adapter
pypolycall client --host localhost --port 8084

# Show protocol information
pypolycall info
```

## Architecture

### Adapter Pattern Compliance

```
User Python Code → PyPolyCall Binding → polycall.exe Runtime → Execution
```

**Key Points:**
- PyPolyCall = **Adapter Only**
- polycall.exe = **Execution Engine**
- User Code = **Logic Declarations**

### State Machine Flow

```
INIT → HANDSHAKE → AUTH → READY → EXECUTING → READY
```

Every state transition requires protocol validation through `polycall.exe`.

## API Reference

### PolyCallClient

Main adapter class for connecting to polycall.exe runtime.

```python
from pypolycall import PolyCallClient

client = PolyCallClient(
    host="localhost",
    port=8084,
    config={
        "zero_trust": True,
        "telemetry_enabled": True,
        "crypto_seed": True
    }
)
```

**Methods:**
- `connect()` - Connect to polycall.exe runtime
- `authenticate(credentials)` - Zero-trust authentication
- `handler(route)` - Register user logic (decorator)
- `execute(route, data)` - Execute through protocol
- `shutdown()` - Clean shutdown

### ProtocolBinding

Low-level protocol adapter.

```python
from pypolycall.core import ProtocolBinding

binding = ProtocolBinding(
    polycall_host="localhost",
    polycall_port=8084
)

await binding.connect()
await binding.authenticate(credentials)
```

## Configuration

### .polycallrc

```yaml
# PyPolyCall binding configuration
binding:
  type: "python"
  version: "1.0.0"
  
runtime:
  host: "localhost"
  port: 8084
  
security:
  zero_trust: true
  crypto_seed: true
  
telemetry:
  enabled: true
  silent_observation: true
```

## Zero-Trust Security

PyPolyCall implements zero-trust architecture:

1. **Cryptographic Seeding** - All state changes use crypto seeds
2. **Continuous Validation** - Every action verified by polycall.exe
3. **State Machine Binding** - NFA mapping for user interactions
4. **Permission Checking** - Hierarchical authorization system

## Telemetry Integration

Silent protocol observation for debugging:

```python
from pypolycall.telemetry import TelemetryObserver

observer = TelemetryObserver()
observer.enable_state_tracking()
observer.enable_request_tracing()
observer.enable_error_capture()
```

## Protocol Compliance

### ✅ Required Behaviors

- Connect to polycall.exe before any operations
- Submit all user logic through handler declarations
- Never bypass protocol validation
- Maintain state machine compliance
- Enable zero-trust verification

### ❌ Prohibited Behaviors

- Direct HTTP server creation
- Bypassing polycall.exe runtime
- State manipulation without protocol
- Disabling security verification
- Cache or store state locally

## Development

### Running Tests

```bash
pytest tests/
pytest tests/integration/ --require-runtime
```

### Protocol Testing

```bash
# Test protocol compliance
pypolycall test --host localhost --port 8084

# Validate state machine
python -m pypolycall.tests.state_machine

# Check zero-trust compliance  
python -m pypolycall.tests.security
```

## License

MIT License - LibPolyCall Trial v1

## Support

- **Documentation**: https://docs.obinexuscomputing.com/libpolycall
- **Issues**: https://gitlab.com/obinexuscomputing/libpolycall-v1trial/-/issues
- **Protocol Questions**: Contact OBINexusComputing

---

**Remember: PyPolyCall is an ADAPTER, not an engine. All execution flows through polycall.exe runtime.**

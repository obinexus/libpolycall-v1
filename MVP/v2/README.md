# libpolycall MVP v2

**Polymorphic Function Call Library - Production Release**  
**Version:** 2.0.0-MVP  
**Status:** Active Development  
**Architecture:** OBINexus Infrastructure as a Service (IaaS)

---

## 🎯 Overview

libpolycall v2 is the core polymorphic binding layer for the OBINexus ecosystem, enabling seamless cross-language function calls with automatic service discovery, fault tolerance, and bidirectional updates. This MVP demonstrates production-ready infrastructure for NLM-Atlas, NLM-Framework, and SemVerX integration.

### Key Innovation

Traditional FFI requires static bindings and manual version management.  
libpolycall v2 provides **dynamic, hot-swappable bindings** with automatic fault recovery and real-time service discovery.

---

## 📁 Directory Structure

```
MVP/v2/
├── include/                    # Header files
│   ├── network.h               # Network protocol definitions
│   ├── polycall.h              # Core API interface
│   ├── polycall_file_parser.h  # Configuration file parsing
│   ├── polycall_micro.h        # Microservice integration
│   ├── polycall_parser.h       # Generic parser interface
│   ├── polycall_protocol.h     # Protocol definitions (HTTP/HTTPS/TLS)
│   ├── polycall_state_machine.h # State management for updates
│   ├── polycall_token.h        # Token-based authentication
│   └── polycall_tokenizer.h    # Configuration tokenizer
│
├── build/                      # Build artifacts
│   └── release/
│       ├── bin/               # Executable binaries
│       ├── lib/               # Shared/static libraries (.so/.a)
│       └── obj/               # Object files
│
├── src/                       # Source files
│   └── polycall_core.c       # Core implementation
│
├── libpolycall-api.js         # JavaScript/Node.js binding
├── libpolycall-config.txt     # Configuration template
├── Makefile                   # Build system
└── README.md                  # This file
```

---

## 🚀 Quick Start

### Building from Source

```bash
# Navigate to MVP/v2 directory
cd /obinexus/workspace/libpolycall/MVP/v2

# Build release version with detached tracing
make release TRACE_DETACHED=1

# Build outputs will be in build/release/
ls build/release/lib/
# libpolycall.so  libpolycall.a
```

### Installation

```bash
# Install to system (requires sudo)
sudo make install

# Verify installation
ldconfig -p | grep libpolycall
```

### Basic Usage (C)

```c
#include <polycall.h>

int main() {
    // Initialize libpolycall
    polycall_context_t* ctx = polycall_init("/etc/obinexus/libpolycall-config.txt");
    
    // Register NLM-Atlas service
    polycall_register_service(ctx, "nlm-atlas", 
                             "https://api.obinexus.org/nlm-atlas",
                             SERVICE_TYPE_IAAS, 
                             BINDING_NATIVE_FFI);
    
    // Discover service by capability
    char* service_url;
    polycall_nlm_discover(ctx, "image.resize", 0.005, &service_url);
    
    // Cleanup
    polycall_shutdown(ctx);
    return 0;
}
```

### JavaScript Binding

```javascript
// Using libpolycall-api.js
const libpolycall = require('./libpolycall-api.js');

// Initialize
const polycall = new libpolycall.PolycallContext();

// Service discovery via NLM-Atlas
const imageService = await polycall.discover({
    capability: 'image.resize',
    maxCost: 0.005
});

// Use the discovered service
const result = await imageService.process(imageData);
```

---

## 🔧 Configuration

### libpolycall-config.txt Format

```ini
# Core Configuration
[system]
version=2.0.0
protocol=https
security_model=OSI-7-Layer
trace_mode=detached

[services]
nlm-atlas=https://api.obinexus.org/nlm-atlas
nlm-framework=https://api.obinexus.org/nlm-framework
semverx=https://api.obinexus.org/semverx
geocall=https://geo.obinexus.org/v1

[updates]
strategy=bidirectional
check_interval=3600
auto_update=true

[plugins]
directory=/usr/local/lib/obinexus/plugins
auto_load=true

[security]
tls_version=1.3
certificate_path=/etc/obinexus/certs/
```

---

## 📦 Components

### Core Headers

| Header | Purpose |
|--------|---------|
| `polycall.h` | Main API interface |
| `network.h` | Network operations (HTTP/HTTPS) |
| `polycall_protocol.h` | Protocol definitions |
| `polycall_state_machine.h` | State management for hot-swapping |
| `polycall_micro.h` | Microservice architecture support |
| `polycall_parser.h` | Configuration parsing |
| `polycall_token.h` | Authentication tokens |
| `polycall_tokenizer.h` | Config file tokenization |

### State Machine

The state machine (`polycall_state_machine.h`) manages:
- **IDLE**: Waiting for requests
- **DISCOVERING**: Finding services via NLM-Atlas
- **BINDING**: Creating FFI bindings
- **EXECUTING**: Running polymorphic calls
- **UPDATING**: Bidirectional update in progress
- **FAULT**: Error recovery mode

### Protocol Support

- **HTTP/HTTPS**: RESTful service communication
- **TLS 1.3**: Secure connections
- **WebSocket**: Real-time updates
- **gRPC**: High-performance RPC (planned)

---

## 🔄 Build Options

```bash
# Debug build with symbols
make debug

# Release build optimized
make release

# Build with AddressSanitizer
make asan

# Build with specific features
make release \
  TRACE_DETACHED=1 \
  ENABLE_PLUGINS=1 \
  WITH_TLS=1

# Clean build artifacts
make clean

# Run tests
make test
```

---

## 🧪 Testing

```bash
# Unit tests
make test-unit

# Integration tests with NLM-Atlas
make test-integration

# Fault injection testing
make test-fault

# Performance benchmarks
make benchmark
```

---

## 🔌 Plugin Development

Create custom plugins that use libpolycall:

```c
// myplugin.c
#include <polycall.h>

POLYCALL_PLUGIN_EXPORT int plugin_init(polycall_context_t* ctx) {
    // Register plugin services
    polycall_register_service(ctx, "myplugin", 
                             "internal://myplugin",
                             SERVICE_TYPE_PLUGIN,
                             BINDING_NATIVE_FFI);
    return 0;
}
```

Compile:
```bash
gcc -shared -fPIC -o myplugin.so myplugin.c -lpolycall
cp myplugin.so /usr/local/lib/obinexus/plugins/
```

---

## 🚦 API Reference

### Initialization
```c
polycall_context_t* polycall_init(const char* config_file);
void polycall_shutdown(polycall_context_t* ctx);
```

### Service Management
```c
polycall_error_t polycall_register_service(...);
polycall_service_t* polycall_get_service(...);
polycall_error_t polycall_invoke_service(...);
```

### NLM Integration
```c
polycall_error_t polycall_nlm_discover(...);     // NLM-Atlas
polycall_error_t polycall_semverx_resolve(...);  // SemVerX
polycall_error_t polycall_geocall_locate(...);   // GeoCall
```

### Fault Tracing
```c
void polycall_enable_tracing(ctx, true);  // Detached mode
void polycall_set_fault_handler(ctx, callback);
```

---

## 🐛 Debugging

Enable verbose logging:
```bash
export POLYCALL_LOG_LEVEL=DEBUG
export POLYCALL_TRACE=1
./your_app
```

View trace logs:
```bash
tail -f /var/log/obinexus/libpolycall.log
```

Analyze core dumps:
```bash
gdb ./app core
(gdb) bt
(gdb) info registers
```

---

## 🔗 Integration Examples

### With NLM-Atlas
```c
// Discover cheapest service dynamically
char* url;
polycall_nlm_discover(ctx, "compute.gpu", 0.01, &url);
```

### With SemVerX
```c
// Resolve compatible versions
char* version;
polycall_semverx_resolve(ctx, "tensorflow", ">=2.0.0-stable", &version);
```

### With GeoCall
```c
// Get location-based services
void* vox_handle;
polycall_geocall_locate(ctx, 37.7749, -122.4194, &vox_handle);
```

---

## 📊 Performance

| Operation | Latency | Throughput |
|-----------|---------|------------|
| Service Discovery | <10ms | 10K req/s |
| FFI Binding | <1ms | 100K calls/s |
| Hot-swap Update | <50ms | N/A |
| Fault Recovery | <100ms | N/A |

---

## 🛣️ Roadmap

- [x] Core polymorphic binding system
- [x] NLM-Atlas integration
- [x] Fault tracing (detached mode)
- [x] JavaScript binding
- [ ] Python ctypes binding
- [ ] Rust FFI binding
- [ ] WebAssembly support
- [ ] gRPC protocol
- [ ] Distributed tracing

---

## 📝 License

Apache 2.0 with OBINexus Patent Pledge

---

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing`)
5. Open Pull Request

---

## 📞 Support

- **Documentation**: [docs.obinexus.org/libpolycall](https://docs.obinexus.org/libpolycall)
- **Issues**: [github.com/obinexus/libpolycall/issues](https://github.com/obinexus/libpolycall/issues)
- **Discord**: [discord.gg/obinexus](https://discord.gg/obinexus)

---

**libpolycall v2 MVP** - The future of polymorphic infrastructure.  
*One binding to rule them all.*
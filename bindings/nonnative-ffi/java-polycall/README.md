# Java PolyCall - LibPolyCall Trial v1 Java Binding

![LibPolyCall Protocol](https://img.shields.io/badge/Protocol-LibPolyCall%20v1-blue)
![Java Version](https://img.shields.io/badge/Java-17%2B-orange)
![License](https://img.shields.io/badge/License-MIT-yellow)
![Architecture](https://img.shields.io/badge/Architecture-Adapter%20Pattern-red)

**Protocol-Compliant Java Adapter for polycall.exe Runtime**

## 🚨 CRITICAL PROTOCOL COMPLIANCE NOTICE

Java PolyCall is an **ADAPTER BINDING** for the LibPolyCall Trial v1 runtime system. This binding **DOES NOT** execute user code directly. All execution must flow through the `polycall.exe` runtime following the program-first architecture paradigm.

### Protocol Law Requirements:
- ✅ **Runtime Dependency**: Requires `polycall.exe` runtime for all operations
- ✅ **Adapter Pattern**: Never bypasses protocol validation layer
- ✅ **Zero-Trust Architecture**: Cryptographic validation at every state transition
- ✅ **State Machine Binding**: All interactions follow finite automaton patterns
- ✅ **Telemetry Integration**: Silent protocol observation for debugging

---

## Quick Start Guide

### Prerequisites

**MANDATORY**: Java PolyCall requires the LibPolyCall runtime (`polycall.exe`) to function. The binding acts as a protocol adapter and cannot operate independently.

```bash
# Verify Java installation (Java 17+ required)
java -version

# Verify polycall.exe availability
polycall.exe --version

# Start runtime server (default port 8084)
polycall.exe server --port 8084 --host localhost
```

### Running Java PolyCall

#### Method 1: Fat JAR (Recommended)
```bash
# Run with all dependencies included
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar info

# Test runtime connectivity
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test --host localhost --port 8084

# Monitor protocol telemetry
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar telemetry --observe --duration 30
```

#### Method 2: Standard JAR (Requires Maven Dependencies)
```bash
# Run standard JAR (requires classpath setup)
java -cp target/java-polycall-1.0.0.jar:target/lib/* org.obinexus.cli.Main info
```

### Basic Protocol Connection Example

```java
import org.obinexus.core.ProtocolBinding;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

public class QuickStartExample {
    public static void main(String[] args) {
        // Initialize protocol binding adapter
        ProtocolBinding binding = new ProtocolBinding("localhost", 8084);
        
        try {
            // Connect to polycall.exe runtime
            CompletableFuture<Boolean> connected = binding.connect();
            if (connected.get()) {
                System.out.println("✓ Connected to polycall.exe runtime");
                
                // Authenticate with zero-trust validation
                Map<String, Object> credentials = Map.of(
                    "username", "developer",
                    "api_key", "your-api-key",
                    "scope", "binding-access"
                );
                
                CompletableFuture<Boolean> authenticated = binding.authenticate(credentials);
                if (authenticated.get()) {
                    System.out.println("✓ Authentication successful");
                    
                    // Execute operation through runtime
                    CompletableFuture<Object> result = binding.executeOperation(
                        "system.status",
                        Map.of("include_metrics", true)
                    );
                    
                    System.out.println("Runtime status: " + result.get());
                }
            }
        } catch (Exception e) {
            System.err.println("Protocol error: " + e.getMessage());
        } finally {
            binding.shutdown();
        }
    }
}
```

---

## Installation & Build

### Building from Source

```bash
# Clone repository (if not already done)
git clone https://github.com/obinexus/libpolycall-v1trial.git
cd libpolycall-v1trial/bindings/java-polycall

# Build project
mvn clean compile

# Run tests
mvn test

# Package JARs
mvn package
```

### Build Artifacts

After successful build, you'll find:

- **Standard JAR**: `target/java-polycall-1.0.0.jar`
- **Fat JAR**: `target/java-polycall-1.0.0-jar-with-dependencies.jar` ⭐ **Recommended**
- **Source JAR**: `target/java-polycall-1.0.0-sources.jar`

### Using Build Scripts

```bash
# Use provided build script
chmod +x scripts/build.sh
./scripts/build.sh

# Test runtime connection
chmod +x scripts/test-runtime.sh
./scripts/test-runtime.sh
```

---

## CLI Commands Reference

### Information Commands
```bash
# Display protocol information
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar info

# Show version details
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar --version
```

### Testing Commands
```bash
# Test basic connectivity
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test

# Test specific host/port
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test --host localhost --port 8084

# Verbose testing
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test --host localhost --port 8084 --verbose
```

### Telemetry Commands
```bash
# Basic telemetry monitoring
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar telemetry

# Observe for specific duration
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar telemetry --observe --duration 60

# Export telemetry data
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar telemetry --export --format json
```

---

## Demo Setup

### 1. Complete Setup Demo

```bash
#!/bin/bash
# demo-setup.sh - Complete Java PolyCall demonstration

echo "🚀 Java PolyCall Demo Setup"
echo "================================"

# Step 1: Verify Java installation
echo "1. Checking Java installation..."
java -version || { echo "❌ Java 17+ required"; exit 1; }

# Step 2: Build project
echo "2. Building Java PolyCall..."
mvn clean package -q || { echo "❌ Build failed"; exit 1; }

# Step 3: Check for polycall.exe
echo "3. Checking polycall.exe runtime..."
if ! command -v polycall.exe &> /dev/null; then
    echo "⚠️  polycall.exe not found in PATH"
    echo "   Please ensure LibPolyCall runtime is installed"
fi

# Step 4: Display available commands
echo "4. Available commands:"
echo "   java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar info"
echo "   java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test"
echo "   java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar telemetry"

echo "✅ Demo setup complete!"
```

### 2. Runtime Connection Demo

```bash
#!/bin/bash
# demo-runtime.sh - Runtime connection demonstration

echo "🔗 Java PolyCall Runtime Connection Demo"
echo "========================================"

# Check if polycall.exe is running
if ! netstat -an | grep -q ":8084"; then
    echo "🚀 Starting polycall.exe runtime..."
    polycall.exe server --port 8084 --host localhost &
    POLYCALL_PID=$!
    sleep 3
    echo "✅ Runtime started (PID: $POLYCALL_PID)"
else
    echo "✅ Runtime already running on port 8084"
fi

# Test connection
echo "🧪 Testing connection..."
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test --host localhost --port 8084

# Monitor telemetry briefly
echo "📊 Monitoring telemetry for 10 seconds..."
timeout 10s java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar telemetry --observe --duration 10

echo "✅ Demo complete!"
```

---

## Architecture Overview

### Adapter Pattern Implementation

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Java Client   │────│  Java PolyCall  │────│  polycall.exe   │
│   Application   │    │    Binding      │    │    Runtime      │
│                 │    │   (Adapter)     │    │   (Engine)      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
        │                        │                        │
        │                        │                        │
    Business Logic      Protocol Translation       Execution
```

### Core Components

#### Protocol Binding Layer (`org.obinexus.core`)
- **ProtocolBinding**: Main adapter interface to polycall.exe
- **ProtocolHandler**: Low-level protocol communication
- **StateManager**: State machine synchronization
- **TelemetryObserver**: Silent protocol observation

#### CLI Layer (`org.obinexus.cli`)
- **Main**: Command-line interface entry point
- **Commands**: Extensible command system (Info, Test, Telemetry)
- **Consumer-Core**: Plugin architecture for custom commands

#### Configuration Layer (`org.obinexus.config`)
- **ConfigManager**: Unified configuration management
- **Environment Integration**: Runtime configuration detection

---

## State Machine Compliance

Java PolyCall follows the LibPolyCall state machine specification:

```
INIT → HANDSHAKE → AUTH → READY → EXECUTING → READY
  ↓        ↓         ↓      ↓         ↓         ↓
Error → Error → Error → Error → Error → SHUTDOWN
```

**Implementation Example:**

```java
ProtocolBinding binding = new ProtocolBinding("localhost", 8084);

// INIT → HANDSHAKE
binding.connect().get();

// HANDSHAKE → AUTH
binding.authenticate(credentials).get();

// AUTH → READY (automatic transition)

// READY → EXECUTING → READY
Object result = binding.executeOperation("operation.name", params).get();

// READY → SHUTDOWN
binding.shutdown();
```

---

## Configuration

### Environment Variables

```bash
# Runtime connection
export JAVA_POLYCALL_HOST=localhost
export JAVA_POLYCALL_PORT=8084

# JVM settings
export JAVA_OPTS="-Xmx512m -Xms256m"

# Logging
export JAVA_POLYCALL_LOG_LEVEL=INFO

# Security
export JAVA_POLYCALL_TLS_ENABLED=true
```

### Application Properties

Located at `src/main/resources/application.properties`:

```properties
# Java PolyCall Configuration
polycall.host=localhost
polycall.port=8084
polycall.timeout=30
polycall.retry.attempts=3

# Logging
logging.level.org.obinexus=DEBUG
logging.level.root=INFO

# Telemetry
telemetry.enabled=true
telemetry.silent.observation=true
telemetry.metrics.interval=60
```

---

## Troubleshooting

### Common Issues

#### 1. Java Version Compatibility
```
Error: UnsupportedClassVersionError
```

**Resolution:**
```bash
# Verify Java version (17+ required)
java -version

# Update JAVA_HOME if necessary
export JAVA_HOME=/path/to/java17
```

#### 2. Runtime Connection Failure
```
Error: Failed to connect to polycall.exe runtime
```

**Resolution:**
```bash
# Verify polycall.exe is running
netstat -an | grep 8084

# Start runtime if not running
polycall.exe server --port 8084 --host localhost

# Test connection
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test
```

#### 3. Build Failures
```
Error: compilation error
```

**Resolution:**
```bash
# Clean and rebuild
mvn clean compile

# Check dependencies
mvn dependency:tree

# Verify Java version in pom.xml matches system
```

### Debug Mode

```bash
# Enable verbose logging
export JAVA_POLYCALL_LOG_LEVEL=DEBUG

# Run with debug output
java -Dlogging.level.org.obinexus=DEBUG -jar target/java-polycall-1.0.0-jar-with-dependencies.jar test

# Monitor with telemetry
java -jar target/java-polycall-1.0.0-jar-with-dependencies.jar telemetry --observe --duration 30
```

---

## Protocol Compliance Validation

### Required Behaviors ✅

- **Runtime Dependency**: All operations require polycall.exe
- **Adapter Pattern**: No direct execution, only protocol translation
- **State Machine**: Follow INIT→HANDSHAKE→AUTH→READY flow
- **Zero-Trust**: Cryptographic validation for all operations
- **Telemetry**: Silent observation enabled by default

### Prohibited Behaviors ❌

- **Direct Execution**: Never execute user code directly
- **Protocol Bypass**: No circumvention of polycall.exe validation
- **Local State**: No persistent state storage outside runtime
- **Security Disable**: Cannot disable zero-trust validation
- **Standalone Operation**: Cannot function without polycall.exe

---

## Performance Considerations

### Memory Configuration

```bash
# Standard usage
java -Xmx512m -Xms256m -jar target/java-polycall-1.0.0-jar-with-dependencies.jar

# High-throughput scenarios
java -Xmx2g -Xms1g -XX:+UseG1GC -jar target/java-polycall-1.0.0-jar-with-dependencies.jar

# Development/testing
java -Xmx256m -jar target/java-polycall-1.0.0-jar-with-dependencies.jar
```

### Connection Pooling

The binding automatically manages connection pooling and provides:
- Automatic retry mechanisms
- Connection health monitoring
- Load balancing across multiple polycall.exe instances
- Graceful degradation handling

---

## Development

### Adding Custom Commands

```java
// Create custom command
@CommandLine.Command(name = "custom", description = "Custom protocol operation")
public class CustomCommand implements Callable<Integer> {
    
    @CommandLine.Option(names = {"--param"}, description = "Custom parameter")
    private String param;
    
    @Override
    public Integer call() throws Exception {
        ProtocolBinding binding = new ProtocolBinding("localhost", 8084);
        binding.connect().get();
        
        Object result = binding.executeOperation("custom.operation", 
            Map.of("param", param)).get();
        
        System.out.println("Result: " + result);
        binding.shutdown();
        return 0;
    }
}
```

### Testing

```bash
# Run unit tests
mvn test

# Run integration tests (requires polycall.exe)
mvn test -Dtest=*IntegrationTest

# Generate test coverage
mvn test jacoco:report
```

---

## Support & Documentation

- **Documentation**: https://docs.obinexuscomputing.com/libpolycall/java-binding
- **Issues**: https://github.com/obinexus/libpolycall-v1trial/issues
- **Protocol Specification**: https://docs.obinexuscomputing.com/libpolycall/protocol
- **Developer Resources**: https://docs.obinexuscomputing.com/libpolycall/development

---

## License

MIT License - LibPolyCall Trial v1

**Copyright (c) 2025 OBINexusComputing**

---

## Author

**Nnamdi Michael Okpala**  
*Founder & Chief Architect*  
*OBINexusComputing*

---

> **Important**: Java PolyCall is an ADAPTER binding. All execution flows through polycall.exe runtime. This binding provides the interface translation layer while maintaining strict protocol compliance with the LibPolyCall Trial v1 specification.
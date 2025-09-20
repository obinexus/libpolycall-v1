## Technical Specification: Isomorphic Binding Architecture


### Core Philosophy: "All Bindings Are Drivers"

```
SQUARE (Perfect Binding):
┌────────────┐
│            │  All 4 sides equal
│   Binding  │  = Symmetric process
│            │  = Bidirectional FFI
└────────────┘

RECTANGLE (Driver):
┌──────────────────┐
│                  │  2 pairs of equal sides
│     Driver       │  = Asymmetric interface
│                  │  = Request/Response pairs
└──────────────────┘
```

### Polyglot Interaction Diagram

```mermaid
graph TD
    subgraph "LibPolyCall Core (C)"
        DRIVER[DRIVER Daemon<br/>Port 3005→8085]
        FFI[FFI Layer<br/>libpolycall.so]
    end
    
    subgraph "Language Bindings"
        COBOL[COBOL<br/>cbl-polycall]
        GO[Go<br/>gosilang]
        PY[Python<br/>py-polycall]
        JS[Node.js<br/>node-polycall]
        JAVA[Java<br/>java-polycall]
    end
    
    subgraph "Schema Transform"
        AST[AST Isomorphism<br/>Huffman-AVL]
        IR[Canonical IR]
    end
    
    COBOL -->|JCL/VSAM| FFI
    GO -->|struct{}| FFI
    PY -->|dict/tuple| FFI
    JS -->|JSON| FFI
    JAVA -->|Object| FFI
    
    FFI --> AST
    AST --> IR
    IR --> DRIVER
```

### Isomorphic Transform Rules

For your example `{x: 20.5, y: 70}`:

```c
// Canonical Intermediate Representation (CIR)
typedef struct {
    enum { FLOAT64, INT64, STRING, NESTED } type;
    union {
        double f64;
        int64_t i64;
        char* str;
        void* nested;
    } value;
} CIR_Value;

typedef struct {
    char* key;
    CIR_Value value;
} CIR_Field;

typedef struct {
    CIR_Field* fields;
    size_t field_count;
} CIR_Object;
```

### Language-Specific Mappings

| Language | Native Type | CIR Transform | Driver Format |
|----------|------------|---------------|---------------|
| Python | `{"x": 20.5, "y": 70}` | `CIR_Object` | Binary protocol |
| Go | `struct{X float64; Y int}` | `CIR_Object` | Binary protocol |
| COBOL | `01 POINT. 05 X PIC 9(2)V9. 05 Y PIC 9(2).` | `CIR_Object` | Binary protocol |
| Java | `class Point{Float x; Integer y;}` | `CIR_Object` | Binary protocol |

### Zero-Trust Protocol

Each binding registers with cryptographically-seeded GUID:
```c
typedef struct {
    uint8_t seed[32];  // Cryptographic seed
    uint64_t session;  // Session identifier
    uint32_t sequence; // Message sequence
    uint16_t checksum; // Data integrity
} PolyCall_Header;
```

### Build Orchestration

The unified Makefile ensures all bindings compile to the same ABI:
```makefile
POLYCALL_ABI = -fPIC -shared -Wl,-soname,lib$@.so.1
CANONICAL_IR = -DUSE_CANONICAL_IR=1

%.so: %.c
    $(CC) $(CFLAGS) $(POLYCALL_ABI) $(CANONICAL_IR) $< -o $@
```

### State Machine Mapping

Every cross-language call follows this state progression:
1. **Parse** → Language-specific AST
2. **Transform** → Canonical IR (lossless)
3. **Validate** → Type coercion matrix
4. **Marshal** → Binary protocol
5. **Route** → DRIVER daemon
6. **Unmarshal** → Target language
7. **Execute** → Native invocation
8. **Return** → Reverse transform

This ensures the "square perfect" binding where input/output maintain structural equivalence across all supported languages.

The key insight: by treating all bindings as drivers with paired interfaces (rectangle), we achieve the perfect square of bidirectional communication without data loss.
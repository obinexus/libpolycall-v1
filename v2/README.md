# PolyCall V2 - Proper C Project Structure

## Directory Structure
```
.
├── src/                # All C source files
│   ├── core/          # Core functionality
│   ├── network/       # Network modules
│   ├── protocol/      # Protocol handlers
│   └── state/         # State machine
├── include/           # All header files
│   └── polycall/      # Public headers
├── build/             # Build artifacts (generated)
├── lib/               # Libraries (generated)
├── test/              # Test source code
│   └── src/           # Test implementations
├── docs/              # Documentation
├── scripts/           # Build and utility scripts
└── Makefile           # Main build file
```

## Build System
- Source code: ONLY in src/
- Headers: ONLY in include/
- Build scripts: ONLY handle compilation, NO code generation
- Clean separation of concerns

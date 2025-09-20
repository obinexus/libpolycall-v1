# OBINexus libpolycall v2 - Master Makefile
# Orchestrates CMake and provides direct build paths

# Configuration
BUILD_DIR ?= build
CMAKE ?= cmake
NPROC ?= $(shell nproc 2>/dev/null || echo 4)

# Phony targets
.PHONY: all clean configure build test install verify
.PHONY: static shared modules features
.PHONY: nlink polybuild

# Default target
all: clean configure build verify

# CMake configuration
configure:
	@echo "=== Configuring OBINexus libpolycall v2 ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED=ON \
		-DBUILD_STATIC=ON \
		-DBUILD_MODULES=ON \
		-DBUILD_FEATURES=ON

# Build libraries
build:
	@echo "=== Building libpolycall ==="
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

# Build specific library types
static:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DBUILD_STATIC=ON -DBUILD_SHARED=OFF
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

shared:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DBUILD_STATIC=OFF -DBUILD_SHARED=ON
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

# Build only modules
modules:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DBUILD_MODULES=ON -DBUILD_FEATURES=OFF
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

# Build only features
features:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DBUILD_MODULES=OFF -DBUILD_FEATURES=ON
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

# Clean build artifacts
clean:
	@echo "=== Cleaning build artifacts ==="
	@rm -rf $(BUILD_DIR)
	@find . -name "*.o" -delete
	@find . -name "*.so*" -delete
	@find . -name "*.dylib" -delete
	@find . -name "*.a" -delete

# Verify build outputs
verify:
	@echo "=== Verifying build outputs ==="
	@echo -n "Static library: "
	@if [ -f "$(BUILD_DIR)/lib/libpolycall.a" ]; then \
		echo "✓ $(BUILD_DIR)/lib/libpolycall.a"; \
		ls -lh $(BUILD_DIR)/lib/libpolycall.a; \
	else \
		echo "✗ Missing"; \
	fi
	@echo -n "Shared library: "
	@if [ -f "$(BUILD_DIR)/lib/libpolycall.so" ] || \
	    [ -f "$(BUILD_DIR)/lib/libpolycall.dylib" ] || \
	    [ -f "$(BUILD_DIR)/bin/libpolycall.dll" ]; then \
		echo "✓ Found"; \
		ls -lh $(BUILD_DIR)/lib/libpolycall.* 2>/dev/null || true; \
	else \
		echo "✗ Missing"; \
	fi

# Installation
install: build
	@echo "=== Installing libpolycall ==="
	@$(CMAKE) --install $(BUILD_DIR)

# OBINexus orchestration integration
nlink:
	@echo "=== nlink orchestration ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DOBINEXUS_NLINK=ON
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

polybuild:
	@echo "=== polybuild orchestration ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DOBINEXUS_POLYBUILD=ON
	@$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

# Direct compilation (bypass CMake)
direct:
	@echo "=== Direct compilation (no CMake) ==="
	@bash scripts/direct_build.sh

# Test
test: build
	@echo "=== Running tests ==="
	@cd $(BUILD_DIR) && ctest --verbose

# Help
help:
	@echo "OBINexus libpolycall v2 Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Clean, configure, build, verify (default)"
	@echo "  build     - Build all libraries"
	@echo "  static    - Build static library only"
	@echo "  shared    - Build shared library only"
	@echo "  modules   - Build feature modules (A-Z) only"
	@echo "  features  - Build src features only"
	@echo "  clean     - Remove all build artifacts"
	@echo "  verify    - Check build outputs"
	@echo "  install   - Install libraries and headers"
	@echo "  nlink     - nlink orchestration build"
	@echo "  polybuild - polybuild orchestration build"
	@echo "  direct    - Direct compilation without CMake"
	@echo "  test      - Run test suite"
	@echo "  help      - Show this message"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR - Build directory (default: build)"
	@echo "  CMAKE     - CMake command (default: cmake)"
	@echo "  NPROC     - Parallel jobs (default: auto)"

# OBINexus libpolycall v2 - Main Makefile
# Orchestrates CMake build process

BUILD_DIR = build
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Release

.PHONY: all clean configure build install test debug release

all: configure build

configure:
	@echo "=== Configuring OBINexus libpolycall v2 ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) ..

build: configure
	@echo "=== Building OBINexus libpolycall v2 ==="
	@cd $(BUILD_DIR) && $(MAKE) -j$(shell nproc)
	@echo "=== Build Complete ==="
	@echo "Libraries: $(BUILD_DIR)/lib/"
	@echo "Binaries: $(BUILD_DIR)/bin/"

debug:
	@$(MAKE) CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug" all

release:
	@$(MAKE) CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release" all

install: build
	@echo "=== Installing OBINexus libpolycall v2 ==="
	@cd $(BUILD_DIR) && $(MAKE) install

clean:
	@echo "=== Cleaning build artifacts ==="
	@rm -rf $(BUILD_DIR)
	@rm -f lib/*.a lib/*.so* bin/*
	@echo "Clean complete"

# Create standard build directories
dirs:
	@mkdir -p build/obj build/lib build/bin
	@mkdir -p lib bin

# Show build status
status:
	@echo "=== Build Status ==="
	@echo "Static libraries:"
	@ls -la $(BUILD_DIR)/lib/*.a 2>/dev/null || echo "  None built"
	@echo "Shared libraries:"
	@ls -la $(BUILD_DIR)/lib/*.so* 2>/dev/null || echo "  None built"
	@echo "Executables:"
	@ls -la $(BUILD_DIR)/bin/* 2>/dev/null || echo "  None built"

# Verify build
verify: build
	@echo "=== Verifying Build ==="
	@file $(BUILD_DIR)/lib/libpolycall.* 2>/dev/null || echo "Libraries not found"
	@ldd $(BUILD_DIR)/lib/libpolycall.so 2>/dev/null || echo "Shared library check skipped"
	@nm $(BUILD_DIR)/lib/libpolycall.a 2>/dev/null | head -5 || echo "Static library check skipped"

# Quick test build
test-build:
	@echo "=== Test Build ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DBUILD_CLI=OFF ..
	@cd $(BUILD_DIR) && $(MAKE) -j2
	@ls -la $(BUILD_DIR)/lib/

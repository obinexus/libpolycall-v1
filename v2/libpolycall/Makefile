# OBINexus libpolycall v2 - Master Makefile
# Wraps CMake build for nlink/polybuild orchestration

BUILD_DIR ?= build
CMAKE ?= cmake
FEATURES ?= all

.PHONY: all clean configure build install test

all: configure build

configure:
	@echo "=== Configuring OBINexus libpolycall v2 ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=ON \
		-DBUILD_STATIC_LIBS=ON \
		-DENABLE_FEATURES=ON

build: configure
	@echo "=== Building libpolycall ==="
	@$(CMAKE) --build $(BUILD_DIR) --parallel

install: build
	@echo "=== Installing libpolycall ==="
	@$(CMAKE) --install $(BUILD_DIR)

clean:
	@echo "=== Cleaning build artifacts ==="
	@rm -rf $(BUILD_DIR)

# Direct make targets (bypass CMake)
native-build:
	@echo "=== Native build (no CMake) ==="
	$(MAKE) -f Makefile.native

# Feature-specific builds
feature-%:
	@echo "=== Building feature: $* ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DPOLYCALL_FEATURE_ONLY=$*
	@$(CMAKE) --build $(BUILD_DIR)

# Library output verification
verify:
	@echo "=== Verifying library outputs ==="
	@if [ -f "$(BUILD_DIR)/libpolycall.so" ] || [ -f "$(BUILD_DIR)/libpolycall.dylib" ]; then \
		echo "✓ Shared library built"; \
	else \
		echo "✗ Shared library missing"; \
	fi
	@if [ -f "$(BUILD_DIR)/libpolycall.a" ]; then \
		echo "✓ Static library built"; \
	else \
		echo "✗ Static library missing"; \
	fi

# Integration with nlink/polybuild
nlink:
	@echo "=== nlink orchestration ==="
	@./scripts/nlink-integrate.sh

polybuild:
	@echo "=== polybuild orchestration ==="
	@./scripts/polybuild-run.sh

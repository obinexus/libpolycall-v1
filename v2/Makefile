# OBINexus libpolycall v2 - Master Makefile
# Builds libraries into build/lib/, objects into build/obj/

BUILD_DIR = build
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Release \
              -DBUILD_SHARED_LIBS=ON \
              -DBUILD_STATIC_LIBS=ON \
              -DBUILD_MODULES=ON \
              -DBUILD_CLI=ON

.PHONY: all clean configure build modules status

all: clean configure build status

configure:
	@echo "=== Configuring libpolycall v2 ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) ..

build:
	@echo "=== Building all targets ==="
	@cd $(BUILD_DIR) && make -j$(shell nproc)

modules: configure
	@echo "=== Building module libraries ==="
	@cd $(BUILD_DIR) && make -j$(shell nproc)
	@echo "Module libraries built:"
	@ls -la $(BUILD_DIR)/lib/*.a $(BUILD_DIR)/lib/*.so 2>/dev/null || true

clean:
	@echo "=== Cleaning build directory ==="
	@rm -rf $(BUILD_DIR)

status:
	@echo ""
	@echo "╔════════════════════════════════════════════════╗"
	@echo "║            Build Status Report                 ║"
	@echo "╚════════════════════════════════════════════════╝"
	@echo "Static Libraries (.a):"
	@ls -lh $(BUILD_DIR)/lib/*.a 2>/dev/null || echo "  None built"
	@echo ""
	@echo "Shared Libraries (.so):"
	@ls -lh $(BUILD_DIR)/lib/*.so* 2>/dev/null || echo "  None built"
	@echo ""
	@echo "Executables:"
	@ls -lh $(BUILD_DIR)/bin/* 2>/dev/null || echo "  None built"
	@echo ""
	@echo "Build structure:"
	@tree -L 2 $(BUILD_DIR) 2>/dev/null || ls -la $(BUILD_DIR)/

quick: clean
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DBUILD_MODULES=OFF .. && make -j2
	@echo "Quick build complete - main library only"

# Target for each module
core:
	@echo "Building core module..."
	@cd $(BUILD_DIR) && make core_static core_shared

micro:
	@echo "Building micro module..."
	@cd $(BUILD_DIR) && make micro_static micro_shared

adapter:
	@echo "Building adapter module..."
	@cd $(BUILD_DIR) && make adapter_static adapter_shared

socket:
	@echo "Building socket module..."
	@cd $(BUILD_DIR) && make socket_static socket_shared

# OBINexus libpolycall v2 - Feature-Isolated Build System
# Isomorphic XML-to-C mapping with platform independence

CC = gcc
CXX = g++
AR = ar
RANLIB = ranlib

# Build flags
CFLAGS = -Wall -Wextra -std=c11 -pthread -fPIC -fcommon
CXXFLAGS = -Wall -Wextra -std=c++11 -pthread -fPIC
LDFLAGS = -pthread
ARFLAGS = rcs

# Include paths
INCLUDES = -I./include -I./include/libpolycall

# Features list (from features.xml)
FEATURES = core adapter hotwire socket micro nlm stream zero

# Directories
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
STATIC_DIR = $(LIB_DIR)/static
SHARED_DIR = $(LIB_DIR)/shared

# Output files
STATIC_LIB = $(STATIC_DIR)/libpolycall.a
SHARED_LIB = $(SHARED_DIR)/libpolycall.so.2.0.0
SHARED_LINK = $(SHARED_DIR)/libpolycall.so

# Platform detection
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
    PLATFORM = linux
    DLL_EXT = so
endif
ifeq ($(UNAME),Darwin)
    PLATFORM = macos
    DLL_EXT = dylib
endif
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    DLL_EXT = dll
endif

# Default target
all: directories $(FEATURES) link-all

# Create directories
directories:
	@mkdir -p $(STATIC_DIR) $(SHARED_DIR)
	@for feature in $(FEATURES); do \
		mkdir -p $(OBJ_DIR)/$$feature; \
	done

# Feature build rules
.PHONY: $(FEATURES)

core: directories
	@echo "Building feature: core"
	@$(MAKE) -C src/core OBJ_DIR=$(PWD)/$(OBJ_DIR)/core \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

adapter: core
	@echo "Building feature: adapter"
	@$(MAKE) -C src/adapter OBJ_DIR=$(PWD)/$(OBJ_DIR)/adapter \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

hotwire: core adapter
	@echo "Building feature: hotwire"
	@$(MAKE) -C src/hotwire OBJ_DIR=$(PWD)/$(OBJ_DIR)/hotwire \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

socket: core
	@echo "Building feature: socket"
	@$(MAKE) -C src/socket OBJ_DIR=$(PWD)/$(OBJ_DIR)/socket \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

micro: core socket
	@echo "Building feature: micro"
	@$(MAKE) -C src/micro OBJ_DIR=$(PWD)/$(OBJ_DIR)/micro \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

nlm: core socket
	@echo "Building feature: nlm"
	@$(MAKE) -C src/nlm OBJ_DIR=$(PWD)/$(OBJ_DIR)/nlm \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

stream: core socket
	@echo "Building feature: stream"
	@$(MAKE) -C src/stream OBJ_DIR=$(PWD)/$(OBJ_DIR)/stream \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

zero: core
	@echo "Building feature: zero"
	@$(MAKE) -C src/zero OBJ_DIR=$(PWD)/$(OBJ_DIR)/zero \
		LIB_DIR=$(PWD)/$(LIB_DIR) INCLUDES="$(INCLUDES)" CC="$(CC)" CFLAGS="$(CFLAGS)"

# Link all features into single library
link-all:
	@echo "Linking all features into libpolycall..."
	@# Static library
	@find $(OBJ_DIR) -name "*.o" -type f | xargs $(AR) $(ARFLAGS) $(STATIC_LIB)
	@$(RANLIB) $(STATIC_LIB)
	@echo "✓ Built $(STATIC_LIB)"
	@# Shared library
	@$(CC) -shared -Wl,-soname,libpolycall.so.2 -o $(SHARED_LIB) \
		$$(find $(OBJ_DIR) -name "*.o" -type f) $(LDFLAGS)
	@ln -sf libpolycall.so.2.0.0 $(SHARED_DIR)/libpolycall.so.2
	@ln -sf libpolycall.so.2 $(SHARED_LINK)
	@echo "✓ Built $(SHARED_LIB)"

# Platform-specific builds
linux:
	@echo "Building for Linux platform..."
	CONFIG=config/platform/linux.xml $(MAKE) all

macos:
	@echo "Building for macOS platform..."
	CONFIG=config/platform/macos.xml $(MAKE) all

windows:
	@echo "Building for Windows platform..."
	CONFIG=config/platform/windows.xml $(MAKE) all

# Install
install: all
	@echo "Installing libpolycall..."
	@# Install headers
	@for feature in $(FEATURES); do \
		install -d /usr/local/include/libpolycall/$$feature; \
		install -m 644 include/libpolycall/$$feature/*.h \
			/usr/local/include/libpolycall/$$feature/ 2>/dev/null || true; \
	done
	@# Install libraries
	install -m 644 $(STATIC_LIB) /usr/local/lib/
	install -m 755 $(SHARED_LIB) /usr/local/lib/
	cd /usr/local/lib && \
		ln -sf libpolycall.so.2.0.0 libpolycall.so.2 && \
		ln -sf libpolycall.so.2 libpolycall.so
	ldconfig
	@echo "✓ Installation complete"

# Clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@for feature in $(FEATURES); do \
		$(MAKE) -C src/$$feature clean 2>/dev/null || true; \
	done
	@echo "✓ Clean complete"

# Test
test: all
	@echo "Running tests..."
	@for feature in $(FEATURES); do \
		if [ -d "test/unit/$$feature" ]; then \
			echo "Testing $$feature..."; \
			$(MAKE) -C test/unit/$$feature test || exit 1; \
		fi; \
	done
	@echo "✓ All tests passed"

# Verify structure
verify:
	@echo "=== libpolycall v2 Structure Verification ==="
	@echo "Features enabled:"
	@for feature in $(FEATURES); do \
		if [ -d "src/$$feature" ]; then \
			echo "  ✓ $$feature"; \
		else \
			echo "  ✗ $$feature (missing)"; \
		fi; \
	done
	@echo ""
	@echo "Libraries built:"
	@ls -lh $(LIB_DIR)/*/* 2>/dev/null || echo "  No libraries built yet"
	@echo ""
	@echo "Platform: $(PLATFORM)"
	@echo "Library extension: $(DLL_EXT)"

# Help
help:
	@echo "OBINexus libpolycall v2 - Feature-Isolated Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build all enabled features"
	@echo "  <feature>  - Build specific feature ($(FEATURES))"
	@echo "  link-all   - Link all features into single library"
	@echo "  install    - Install headers and libraries"
	@echo "  clean      - Remove all build artifacts"
	@echo "  test       - Run test suite"
	@echo "  verify     - Verify structure and build"
	@echo ""
	@echo "Platform builds:"
	@echo "  linux      - Build for Linux"
	@echo "  macos      - Build for macOS"
	@echo "  windows    - Build for Windows"
	@echo ""
	@echo "Features can be built independently:"
	@echo "  make core      # Build only core feature"
	@echo "  make adapter   # Build adapter (and dependencies)"
	@echo "  make hotwire   # Build hotwire (and dependencies)"

.PHONY: all directories $(FEATURES) link-all install clean test verify help
.PHONY: linux macos windows

# ============================================
# Namespace System Rules
# ============================================

namespace: directories
	@echo "Building namespace resolver..."
	@$(CC) $(CFLAGS) -c namespace/src/hash_table.c -o $(OBJ_DIR)/hash_table.o
	@$(AR) rcs $(STATIC_DIR)/libpolycall_namespace.a $(OBJ_DIR)/hash_table.o
	@$(CC) -shared -o $(SHARED_DIR)/libpolycall_namespace.so $(OBJ_DIR)/hash_table.o
	@echo "✓ Namespace system built"

# Build modules matching pattern
modules-pattern-%:
	@pattern=$$(echo $* | sed 's/-/\//g'); \
	for module in $$(find modules -path "*$$pattern*" -name "*.c"); do \
		obj=$$(echo $$module | sed 's/\.c/.o/'); \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$module -o $$obj; \
		echo "✓ Built $$module"; \
	done

# Build all modules
modules-all:
	@find modules -name "*.c" | while read module; do \
		obj=$$(echo $$module | sed 's/\.c/.o/'); \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$module -o $$obj; \
		base=$$(basename $$module .c); \
		dir=$$(dirname $$module); \
		$(AR) rcs $$dir/$$base.a $$obj; \
		$(CC) -shared -o $$dir/$$base.so $$obj $(LDFLAGS); \
		echo "✓ Built $$base (.a/.so)"; \
	done

.PHONY: namespace modules-pattern-% modules-all

# ============================================
# Namespace System Rules
# ============================================

namespace: directories
	@echo "Building namespace resolver..."
	@$(CC) $(CFLAGS) -c namespace/src/hash_table.c -o $(OBJ_DIR)/hash_table.o
	@$(AR) rcs $(STATIC_DIR)/libpolycall_namespace.a $(OBJ_DIR)/hash_table.o
	@$(CC) -shared -o $(SHARED_DIR)/libpolycall_namespace.so $(OBJ_DIR)/hash_table.o
	@echo "✓ Namespace system built"

# Build modules matching pattern
modules-pattern-%:
	@pattern=$$(echo $* | sed 's/-/\//g'); \
	for module in $$(find modules -path "*$$pattern*" -name "*.c"); do \
		obj=$$(echo $$module | sed 's/\.c/.o/'); \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$module -o $$obj; \
		echo "✓ Built $$module"; \
	done

# Build all modules
modules-all:
	@find modules -name "*.c" | while read module; do \
		obj=$$(echo $$module | sed 's/\.c/.o/'); \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$module -o $$obj; \
		base=$$(basename $$module .c); \
		dir=$$(dirname $$module); \
		$(AR) rcs $$dir/$$base.a $$obj; \
		$(CC) -shared -o $$dir/$$base.so $$obj $(LDFLAGS); \
		echo "✓ Built $$base (.a/.so)"; \
	done

.PHONY: namespace modules-pattern-% modules-all

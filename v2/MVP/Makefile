# libpolycall v2 Makefile
# OBINexus Polymorphic Function Call Library Build System

# Compiler and flags
CC = gcc
CXX = g++
AR = ar
RUST = cargo

# Version info
VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Base flags
CFLAGS = -Wall -Wextra -fPIC -O2 -std=c11 \
         -DLIBPOLYCALL_VERSION_MAJOR=$(VERSION_MAJOR) \
         -DLIBPOLYCALL_VERSION_MINOR=$(VERSION_MINOR) \
         -DLIBPOLYCALL_VERSION_PATCH=$(VERSION_PATCH) \
         -DLIBPOLYCALL_EXPORTS

# Debug/Release modes
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0 -DDEBUG -fsanitize=address
    BUILD_DIR = build/debug
else
    CFLAGS += -O3 -DNDEBUG
    BUILD_DIR = build/release
endif

# Detached tracing support
TRACE_DETACHED ?= 1
ifeq ($(TRACE_DETACHED), 1)
    CFLAGS += -DTRACE_DETACHED -rdynamic
    LDFLAGS += -ldl -lbacktrace
endif

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
    SHARED_EXT = .so
    LDFLAGS += -lrt -lpthread -lssl -lcrypto
    CFLAGS += -DLINUX
endif
ifeq ($(UNAME_S),Darwin)
    PLATFORM = darwin
    SHARED_EXT = .dylib
    LDFLAGS += -framework Security -framework CoreFoundation
    CFLAGS += -DDARWIN
endif

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin
TEST_DIR = tests
PLUGIN_DIR = plugins

# Output files
STATIC_LIB = $(LIB_DIR)/libpolycall.a
SHARED_LIB = $(LIB_DIR)/libpolycall$(SHARED_EXT)
TEST_BIN = $(BIN_DIR)/test_polycall

# Source files
SOURCES = \
    $(SRC_DIR)/polycall_core.c \
    $(SRC_DIR)/polycall_service.c \
    $(SRC_DIR)/polycall_binding.c \
    $(SRC_DIR)/polycall_plugin.c \
    $(SRC_DIR)/polycall_update.c \
    $(SRC_DIR)/polycall_trace.c \
    $(SRC_DIR)/polycall_nlm.c \
    $(SRC_DIR)/polycall_semverx.c \
    $(SRC_DIR)/polycall_geocall.c \
    $(SRC_DIR)/polycall_security.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Header files
HEADERS = \
    $(INC_DIR)/libpolycall.h \
    $(INC_DIR)/polycall_internal.h \
    $(INC_DIR)/polycall_ffi.h

# Include paths
INCLUDES = -I$(INC_DIR) -I/usr/local/include

# Phony targets
.PHONY: all clean install test plugins rust-bindings python-bindings docs

# Default target
all: dirs $(STATIC_LIB) $(SHARED_LIB) plugins

# Create directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BIN_DIR)

# Build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build static library
$(STATIC_LIB): $(OBJECTS)
	@echo "  AR      $@"
	@$(AR) rcs $@ $(OBJECTS)

# Build shared library
$(SHARED_LIB): $(OBJECTS)
	@echo "  LD      $@"
	@$(CC) -shared $(OBJECTS) -o $@ $(LDFLAGS)
ifeq ($(PLATFORM),linux)
	@ln -sf libpolycall$(SHARED_EXT) $(LIB_DIR)/libpolycall.so.$(VERSION_MAJOR)
	@ln -sf libpolycall.so.$(VERSION_MAJOR) $(LIB_DIR)/libpolycall.so
endif

# Build plugins
plugins: $(SHARED_LIB)
	@echo "Building plugins..."
	@$(MAKE) -C $(PLUGIN_DIR) BUILD_DIR=../$(BUILD_DIR)

# Build Rust bindings for semverx integration
rust-bindings:
	@echo "Building Rust FFI bindings..."
	@cd rust && $(RUST) build --release
	@cp rust/target/release/libpolycall_sys.a $(LIB_DIR)/
	@cp rust/target/release/libpolycall_sys$(SHARED_EXT) $(LIB_DIR)/

# Install Python bindings
python-bindings: $(SHARED_LIB)
	@echo "Installing Python bindings..."
	@cp python/libpolycall.py $(LIB_DIR)/
	@python3 -m pip install --user $(LIB_DIR)/libpolycall.py

# Build tests
test: $(SHARED_LIB)
	@echo "Building tests..."
	@$(CC) $(CFLAGS) $(INCLUDES) $(TEST_DIR)/test_main.c -o $(TEST_BIN) \
	       -L$(LIB_DIR) -lpolycall $(LDFLAGS)
	@echo "Running tests..."
	@LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH $(TEST_BIN)

# Fault tracing test with detached mode
test-trace: $(SHARED_LIB)
	@echo "Testing detached fault tracing..."
	@$(CC) $(CFLAGS) $(INCLUDES) $(TEST_DIR)/test_trace.c -o $(BIN_DIR)/test_trace \
	       -L$(LIB_DIR) -lpolycall $(LDFLAGS)
	@LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH $(BIN_DIR)/test_trace --detach

# Integration test with NLM-Atlas, SemVerX, GeoCall
test-integration: $(SHARED_LIB)
	@echo "Running integration tests..."
	@python3 $(TEST_DIR)/test_integration.py

# Generate documentation
docs:
	@echo "Generating documentation..."
	@doxygen Doxyfile

# Install to system
install: all
	@echo "Installing libpolycall v$(VERSION)..."
	@install -d $(DESTDIR)/usr/local/lib
	@install -d $(DESTDIR)/usr/local/include
	@install -d $(DESTDIR)/usr/local/lib/obinexus/plugins
	@install -d $(DESTDIR)/etc/obinexus
	@install -m 644 $(SHARED_LIB) $(DESTDIR)/usr/local/lib/
	@install -m 644 $(STATIC_LIB) $(DESTDIR)/usr/local/lib/
	@install -m 644 $(INC_DIR)/libpolycall.h $(DESTDIR)/usr/local/include/
	@install -m 644 config/libpolycall.config.xml $(DESTDIR)/etc/obinexus/
ifeq ($(PLATFORM),linux)
	@ldconfig
endif
	@echo "Installation complete!"

# Uninstall from system
uninstall:
	@echo "Uninstalling libpolycall..."
	@rm -f $(DESTDIR)/usr/local/lib/libpolycall*
	@rm -f $(DESTDIR)/usr/local/include/libpolycall.h
	@rm -rf $(DESTDIR)/usr/local/lib/obinexus/plugins
	@rm -f $(DESTDIR)/etc/obinexus/libpolycall.config.xml

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -rf rust/target
	@rm -f python/*.pyc python/__pycache__

# Deep clean (including downloaded dependencies)
distclean: clean
	@echo "Deep cleaning..."
	@rm -rf vendor/
	@rm -rf .cache/

# Development helpers

# Watch for changes and rebuild
watch:
	@echo "Watching for changes..."
	@while true; do \
	    make -q || make; \
	    sleep 1; \
	done

# Run with Address Sanitizer
asan: DEBUG=1
asan: clean all test

# Run with Thread Sanitizer
tsan: CFLAGS+=-fsanitize=thread
tsan: clean all test

# Valgrind memory check
valgrind: $(TEST_BIN)
	@valgrind --leak-check=full --show-leak-kinds=all \
	          --track-origins=yes --verbose \
	          --log-file=valgrind.log \
	          $(TEST_BIN)
	@echo "Valgrind report saved to valgrind.log"

# Performance profiling
profile: CFLAGS+=-pg
profile: clean all
	@echo "Run your program and then use 'gprof' to analyze"

# Generate compile_commands.json for IDE support
compile_commands:
	@bear -- make clean all

# Package for distribution
package: all docs
	@echo "Creating distribution package..."
	@mkdir -p dist
	@tar czf dist/libpolycall-$(VERSION).tar.gz \
	     --exclude=dist --exclude=build --exclude=.git \
	     .
	@echo "Package created: dist/libpolycall-$(VERSION).tar.gz"

# Docker build for CI/CD
docker-build:
	@docker build -t obinexus/libpolycall:$(VERSION) .
	@docker tag obinexus/libpolycall:$(VERSION) obinexus/libpolycall:latest

# Help target
help:
	@echo "libpolycall v$(VERSION) - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Build library and plugins (default)"
	@echo "  clean           - Remove build artifacts"
	@echo "  install         - Install to system"
	@echo "  uninstall       - Remove from system"
	@echo "  test            - Run tests"
	@echo "  test-trace      - Test detached fault tracing"
	@echo "  test-integration- Run integration tests"
	@echo "  plugins         - Build plugin modules"
	@echo "  rust-bindings   - Build Rust FFI bindings"
	@echo "  python-bindings - Install Python bindings"
	@echo "  docs            - Generate documentation"
	@echo "  package         - Create distribution package"
	@echo "  docker-build    - Build Docker image"
	@echo ""
	@echo "Variables:"
	@echo "  DEBUG=1         - Build with debug symbols"
	@echo "  TRACE_DETACHED=1- Enable detached tracing (default)"
	@echo ""
	@echo "Examples:"
	@echo "  make DEBUG=1 test"
	@echo "  make install DESTDIR=/custom/path"

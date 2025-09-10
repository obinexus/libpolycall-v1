# CBLPolyCall Cross-Platform Makefile v1.0
# OBINexus Aegis Engineering - COBOL FFI Bridge Build System
# Technical Lead: Nnamdi Michael Okpala

# Project configuration
PROJECT_NAME = cblpolycall
VERSION = 1.0.0
SRC_DIR = src
BUILD_DIR = build
TARGET_DIR = target
COPYBOOK_DIR = copybooks

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
    EXT = 
    SHARED_EXT = .so
endif
ifeq ($(UNAME_S),Darwin)
    PLATFORM = macos
    EXT = 
    SHARED_EXT = .dylib
endif
ifneq (,$(findstring CYGWIN,$(UNAME_S)))
    PLATFORM = windows
    EXT = .exe
    SHARED_EXT = .dll
endif
ifneq (,$(findstring MINGW,$(UNAME_S)))
    PLATFORM = windows
    EXT = .exe
    SHARED_EXT = .dll
endif

# Compiler detection and flags
COBOL_COMPILER := $(shell which cobc 2>/dev/null || which cob 2>/dev/null || echo "none")
ifeq ($(COBOL_COMPILER),none)
    $(error No COBOL compiler found. Please install GnuCOBOL or Micro Focus COBOL)
endif

# Build flags with GnuCOBOL version detection
COBOL_VERSION := $(shell cobc --version 2>/dev/null | head -1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+' || echo "0.0.0")
COBOL_MAJOR := $(shell echo $(COBOL_VERSION) | cut -d. -f1)
COBOL_MINOR := $(shell echo $(COBOL_VERSION) | cut -d. -f2)

# Adjust flags based on GnuCOBOL version and platform
ifeq ($(PLATFORM),linux)
    ifeq ($(shell test $(COBOL_MAJOR) -ge 3 && test $(COBOL_MINOR) -ge 2; echo $?),0)
        COBOL_FLAGS = -fPIC
    else
        COBOL_FLAGS = 
    endif
endif
ifeq ($(PLATFORM),macos)
    ifeq ($(shell test $(COBOL_MAJOR) -ge 3 && test $(COBOL_MINOR) -ge 2; echo $?),0)
        COBOL_FLAGS = -fPIC
    else
        COBOL_FLAGS = 
    endif
endif
ifeq ($(PLATFORM),windows)
    COBOL_FLAGS = 
endif

CFLAGS = -O2 -g $(COBOL_FLAGS)
LDFLAGS = -L. -lpolycall

# Source files
MAIN_SRC = $(SRC_DIR)/MAIN.CBL
BRIDGE_SRC = $(SRC_DIR)/POLYCALL.CBL
COPYBOOKS = $(wildcard $(COPYBOOK_DIR)/*.CPY)

# Targets
MAIN_TARGET = $(TARGET_DIR)/cblpolycall$(EXT)
BRIDGE_TARGET = $(BUILD_DIR)/polycall-bridge$(SHARED_EXT)

# Default target
.PHONY: all clean install test package help debug

all: directories $(MAIN_TARGET)

# Debug target for troubleshooting
debug:
	@echo "CBLPolyCall Build Environment Debug Information"
	@echo "=============================================="
	@echo "Platform: $(PLATFORM)"
	@echo "COBOL Compiler: $(COBOL_COMPILER)"
	@echo "COBOL Version: $(COBOL_VERSION)"
	@echo "COBOL Major: $(COBOL_MAJOR)"
	@echo "COBOL Minor: $(COBOL_MINOR)"
	@echo "COBOL Flags: $(COBOL_FLAGS)"
	@echo "C Flags: $(CFLAGS)"
	@echo "LD Flags: $(LDFLAGS)"
	@echo "Shared Extension: $(SHARED_EXT)"
	@echo "Executable Extension: $(EXT)"

# Create build directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(TARGET_DIR)

# Build main executable
$(MAIN_TARGET): $(MAIN_SRC) $(BRIDGE_SRC) $(COPYBOOKS)
	@echo "Building CBLPolyCall executable..."
	@if [ -f "$(shell which cobc)" ]; then \
		cobc -x $(CFLAGS) -I$(COPYBOOK_DIR) -o $@ $(MAIN_SRC) $(BRIDGE_SRC); \
	elif [ -f "$(shell which cob)" ]; then \
		cob -x $(CFLAGS) -I$(COPYBOOK_DIR) -o $@ $(MAIN_SRC) $(BRIDGE_SRC); \
	else \
		echo "Error: COBOL compiler not found"; exit 1; \
	fi
	@echo "Build completed: $@"

# Build shared library
$(BRIDGE_TARGET): $(BRIDGE_SRC) $(COPYBOOKS)
	@echo "Building PolyCall bridge library..."
	@if [ -f "$(shell which cobc)" ]; then \
		cobc -m $(CFLAGS) -I$(COPYBOOK_DIR) -o $@ $(BRIDGE_SRC); \
	elif [ -f "$(shell which cob)" ]; then \
		cob -m $(CFLAGS) -I$(COPYBOOK_DIR) -o $@ $(BRIDGE_SRC); \
	else \
		echo "Error: COBOL compiler not found"; exit 1; \
	fi
	@echo "Bridge library built: $@"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(TARGET_DIR)
	@rm -f *.o *.obj *.lst
	@echo "Clean completed"

# Install to system
install: $(MAIN_TARGET)
	@echo "Installing CBLPolyCall..."
	@sudo cp $(MAIN_TARGET) /usr/local/bin/ 2>/dev/null || cp $(MAIN_TARGET) $(HOME)/bin/ || echo "Manual installation required"
	@echo "Installation completed"

# Run tests
test: $(MAIN_TARGET)
	@echo "Running CBLPolyCall tests..."
	@if [ -f tests/test_runner.sh ]; then \
		cd tests && ./test_runner.sh; \
	else \
		echo "No tests available"; \
	fi

# Create distribution package
package: all
	@echo "Creating distribution package..."
	@mkdir -p dist/$(PROJECT_NAME)-$(VERSION)
	@cp -r $(TARGET_DIR)/* dist/$(PROJECT_NAME)-$(VERSION)/
	@cp README.md dist/$(PROJECT_NAME)-$(VERSION)/ 2>/dev/null || true
	@cd dist && tar -czf $(PROJECT_NAME)-$(VERSION)-$(PLATFORM).tar.gz $(PROJECT_NAME)-$(VERSION)
	@echo "Package created: dist/$(PROJECT_NAME)-$(VERSION)-$(PLATFORM).tar.gz"

# Show help
help:
	@echo "CBLPolyCall Build System v$(VERSION)"
	@echo "Available targets:"
	@echo "  all       - Build all targets (default)"
	@echo "  clean     - Clean build artifacts"
	@echo "  install   - Install to system"
	@echo "  test      - Run tests"
	@echo "  package   - Create distribution package"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Platform: $(PLATFORM)"
	@echo "COBOL Compiler: $(COBOL_COMPILER)"

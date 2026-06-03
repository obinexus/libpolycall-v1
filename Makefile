# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -I./include -fPIC
LDFLAGS := -pthread
EXE_LDFLAGS ?= $(LDFLAGS)
USE_OPENSSL ?= 0

# Platform-specific settings
ifeq ($(OS),Windows_NT)
PLATFORM := windows
LDFLAGS += -lws2_32
CFLAGS += -D_WIN32 -D__USE_MINGW_ANSI_STDIO=1
SHARED_EXT := dll
SHARED_LDFLAGS := -shared
EXE_EXT := .exe
else
UNAME_S := $(shell uname -s 2>/dev/null || echo Unknown)
ifeq ($(UNAME_S),Darwin)
PLATFORM := macos
SHARED_EXT := dylib
SHARED_LDFLAGS := -dynamiclib
EXE_EXT :=
else ifeq ($(UNAME_S),Linux)
PLATFORM := linux
SHARED_EXT := so
SHARED_LDFLAGS := -shared
EXE_EXT :=
else
PLATFORM := unix
SHARED_EXT := so
SHARED_LDFLAGS := -shared
EXE_EXT :=
endif
endif

ifeq ($(USE_OPENSSL),1)
LDFLAGS += -lssl -lcrypto
endif

# Debug/Release flags
DEBUG_FLAGS := -g -DDEBUG
RELEASE_FLAGS := -O2 -DNDEBUG

# Directories
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
LIB_DIR := lib
BIN_DIR := bin

# Source files
MAIN_SRC := $(SRC_DIR)/main.c
SRCS := $(filter-out $(MAIN_SRC),$(wildcard $(SRC_DIR)/*.c))
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Main executable
MAIN_OBJ := $(BUILD_DIR)/main.o
EXECUTABLE := polycall$(EXE_EXT)
DEPS := $(OBJS:.o=.d) $(MAIN_OBJ:.o=.d)

# Library name
LIB_NAME := libpolycall
STATIC_LIB := $(LIB_DIR)/$(LIB_NAME).a
SHARED_LIB := $(LIB_DIR)/$(LIB_NAME).$(SHARED_EXT)

# Installation paths
PREFIX := /usr/local
INSTALL_INC_DIR := $(PREFIX)/include/$(LIB_NAME)
INSTALL_LIB_DIR := $(PREFIX)/lib
INSTALL_BIN_DIR := $(PREFIX)/bin

# Default target
.PHONY: all
all: dirs $(STATIC_LIB) $(SHARED_LIB) $(BIN_DIR)/$(EXECUTABLE)

# Create necessary directories
.PHONY: dirs
ifeq ($(PLATFORM),windows)
dirs:
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@if not exist $(LIB_DIR) mkdir $(LIB_DIR)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
else
dirs:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR)
endif

# Debug build
.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

# Release build
.PHONY: release
release: CFLAGS += $(RELEASE_FLAGS)
release: all

# Static release executable for minimal container runtimes
.PHONY: static
static: static-check
static: CFLAGS += $(RELEASE_FLAGS)
static: EXE_LDFLAGS += -static
static: all

.PHONY: static-check
static-check:
ifeq ($(PLATFORM),linux)
	@echo "Building static executable for Linux"
else ifeq ($(PLATFORM),macos)
	@echo "make static is unsupported on macOS because fully static executables are not available with the standard toolchain"
	@exit 1
else ifeq ($(PLATFORM),windows)
	@$(CC) -dumpmachine 2>NUL | findstr /i "mingw" >NUL || (echo make static requires a MinGW GCC toolchain on Windows & exit 1)
else
	@echo "make static is unsupported on this Unix platform"
	@exit 1
endif

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Compile main executable
$(BUILD_DIR)/main.o: $(MAIN_SRC)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Create static library
$(STATIC_LIB): $(OBJS)
	ar rcs $@ $^

# Create shared library
$(SHARED_LIB): $(OBJS)
	$(CC) $(SHARED_LDFLAGS) -o $@ $^ $(LDFLAGS)

# Link executable
$(BIN_DIR)/$(EXECUTABLE): $(MAIN_OBJ) $(STATIC_LIB)
	$(CC) $(MAIN_OBJ) $(STATIC_LIB) -o $@ $(EXE_LDFLAGS)

# Install (Unix-like systems only)
.PHONY: install
install: all
ifeq ($(PLATFORM),windows)
	@echo "make install is not supported on Windows"
else
	@mkdir -p $(INSTALL_INC_DIR)
	@mkdir -p $(INSTALL_LIB_DIR)
	@mkdir -p $(INSTALL_BIN_DIR)
	cp $(INC_DIR)/*.h $(INSTALL_INC_DIR)
	cp $(STATIC_LIB) $(SHARED_LIB) $(INSTALL_LIB_DIR)
	cp $(BIN_DIR)/$(EXECUTABLE) $(INSTALL_BIN_DIR)
ifeq ($(PLATFORM),linux)
	ldconfig
endif
endif

# Uninstall (Unix-like systems only)
.PHONY: uninstall
uninstall:
ifeq ($(PLATFORM),windows)
	@echo "make uninstall is not supported on Windows"
else
	rm -rf $(INSTALL_INC_DIR)
	rm -f $(INSTALL_LIB_DIR)/$(LIB_NAME).*
	rm -f $(INSTALL_BIN_DIR)/$(EXECUTABLE)
endif

# Clean build files
.PHONY: clean
ifeq ($(PLATFORM),windows)
clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist $(LIB_DIR) rmdir /s /q $(LIB_DIR)
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
else
clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR)
endif

# Clean everything including installed files
.PHONY: distclean
distclean: clean uninstall

# Ensure output directories exist before generating artifacts
$(OBJS) $(MAIN_OBJ) $(STATIC_LIB) $(SHARED_LIB) $(BIN_DIR)/$(EXECUTABLE): | dirs

# Include dependency files
-include $(DEPS)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all        - Build everything (default)"
	@echo "  debug      - Build with debug flags"
	@echo "  release    - Build with release flags"
	@echo "  static     - Build a static executable on supported platforms"
	@echo "  clean      - Remove build files"
	@echo "  install    - Install libraries and headers (unsupported on Windows)"
	@echo "  uninstall  - Remove installed files (unsupported on Windows)"
	@echo "  distclean  - Remove all generated files"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Options:"
	@echo "  USE_OPENSSL=1 - Link libssl/libcrypto when crypto APIs are used"

# PolyCall Makefile - Clean Build System
# NO code generation - only compilation

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11 -pthread -fPIC
CFLAGS += -Iinclude
LDFLAGS = -pthread

# Source directories
SRC_DIRS = src/core src/network src/protocol src/state
TEST_DIR = test/src

# Find all source files
SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJECTS = $(patsubst %.c,build/%.o,$(notdir $(SOURCES)))
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,build/test_%,$(TEST_SOURCES))

# Libraries
STATIC_LIB = lib/libpolycall.a
SHARED_LIB = lib/libpolycall.so

# VPATH for finding source files
VPATH = $(SRC_DIRS)

.PHONY: all clean static shared test

all: static shared

static: $(STATIC_LIB)
shared: $(SHARED_LIB)

# Create build directory
build:
	@mkdir -p build

# Pattern rule for object files
build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Static library
$(STATIC_LIB): $(OBJECTS)
	@mkdir -p lib
	$(AR) rcs $@ $^

# Shared library
$(SHARED_LIB): $(OBJECTS)
	@mkdir -p lib
	$(CC) -shared -o $@ $^ $(LDFLAGS)

# Test executables
build/test_%: $(TEST_DIR)/%.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -o $@ -Llib -lpolycall $(LDFLAGS)

test: $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

clean:
	rm -rf build lib/*.a lib/*.so

info:
	@echo "Source files: $(SOURCES)"
	@echo "Object files: $(OBJECTS)"
	@echo "Test files: $(TEST_SOURCES)"

.SUFFIXES:

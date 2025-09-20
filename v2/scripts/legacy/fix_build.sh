#!/bin/bash
# OBINexus libpolycall v2 - Complete Build Fix
# Resolves duplicate definitions, moves CLI, adds PID tracking

set -e

echo "=== OBINexus Build System Repair ==="
echo "Fixing duplicate definitions and restructuring..."

# 1. Remove ALL CMake test artifacts
echo "[1/7] Removing CMake contamination..."
find . -name "CMakeCCompilerId.c" -delete 2>/dev/null || true
find . -name "CMakeCCompilerId.o" -delete 2>/dev/null || true
find . -name "CMakeCXXCompilerId.cpp" -delete 2>/dev/null || true
find . -type d -name "CMakeFiles" -exec rm -rf {} + 2>/dev/null || true
rm -f CMakeCache.txt 2>/dev/null || true

# 2. Move main.c to src/cli/
echo "[2/7] Restructuring CLI..."
mkdir -p src/cli
if [ -f "src/core/main.c" ]; then
    mv src/core/main.c src/cli/polycall_cli.c
    echo "  Moved main.c to src/cli/polycall_cli.c"
fi

# 3. Remove duplicate source files
echo "[3/7] Removing duplicate sources..."
# Remove duplicate banking service
if [ -f "src/core/micro/polycall_banking_service.c" ] && [ -f "src/micro/polycall_banking_service.c" ]; then
    rm -f src/core/micro/polycall_banking_service.c
    echo "  Removed duplicate polycall_banking_service.c from core/micro"
fi

# Remove duplicate nlm files
if [ -f "src/core/nlm_altas_avl_huffman.c" ]; then
    rm -f src/core/nlm_altas_avl_huffman.c  # typo version
fi
if [ -f "src/core/nlm_atlas_avl_huffman.c" ] && [ -f "src/nlm/atlas_avl_huffman.c" ]; then
    # Keep only one version in src/nlm/
    mv src/core/nlm_atlas_avl_huffman.c src/nlm/nlm_atlas_avl_huffman.c 2>/dev/null || true
    echo "  Consolidated NLM Atlas files"
fi

# 4. Create PID management module
echo "[4/7] Creating PID management..."
mkdir -p src/cli/monitor
cat > src/cli/monitor/pid_manager.c << 'EOF'
// OBINexus PID Manager - Process tracking and death watch
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define PID_FILE "/var/run/polycall.pid"
#define PID_FILE_ALT "/tmp/polycall.pid"
#define MAX_PID_LEN 16

typedef struct {
    pid_t pid;
    time_t start_time;
    char process_name[256];
} ProcessInfo;

static ProcessInfo g_process_info = {0};

// Write PID to file
int write_pid_file(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        // Try alternate location
        fp = fopen(PID_FILE_ALT, "w");
        if (!fp) {
            fprintf(stderr, "Failed to create PID file: %s\n", strerror(errno));
            return -1;
        }
        filename = PID_FILE_ALT;
    }
    
    pid_t pid = getpid();
    fprintf(fp, "%d\n", pid);
    fclose(fp);
    
    g_process_info.pid = pid;
    g_process_info.start_time = time(NULL);
    
    printf("[PID Manager] Process %d logged to %s\n", pid, filename);
    return 0;
}

// Check if process is running
int check_process_alive(pid_t pid) {
    return kill(pid, 0) == 0 ? 1 : 0;
}

// Read PID from file
pid_t read_pid_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fp = fopen(PID_FILE_ALT, "r");
        if (!fp) {
            return -1;
        }
    }
    
    char buf[MAX_PID_LEN];
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    return (pid_t)atoi(buf);
}

// Remove PID file on exit
void remove_pid_file(void) {
    unlink(PID_FILE);
    unlink(PID_FILE_ALT);
    printf("[PID Manager] PID file removed\n");
}

// Death watch handler
static void death_watch_handler(int sig) {
    (void)sig;
    printf("[Death Watch] Process %d received signal %d\n", getpid(), sig);
    remove_pid_file();
    exit(0);
}

// Initialize death watch
void init_death_watch(void) {
    signal(SIGTERM, death_watch_handler);
    signal(SIGINT, death_watch_handler);
    signal(SIGHUP, death_watch_handler);
    
    // Register cleanup on normal exit
    atexit(remove_pid_file);
    
    printf("[Death Watch] Initialized for process %d\n", getpid());
}

// Check for stale PID file
int check_stale_pid(void) {
    pid_t old_pid = read_pid_file(PID_FILE);
    if (old_pid == -1) {
        old_pid = read_pid_file(PID_FILE_ALT);
    }
    
    if (old_pid > 0) {
        if (check_process_alive(old_pid)) {
            fprintf(stderr, "Another instance (PID %d) is already running\n", old_pid);
            return 1;
        } else {
            printf("[PID Manager] Removing stale PID file for process %d\n", old_pid);
            remove_pid_file();
        }
    }
    
    return 0;
}

// Initialize PID management
int pid_manager_init(const char* process_name) {
    strncpy(g_process_info.process_name, process_name, 
            sizeof(g_process_info.process_name) - 1);
    
    // Check for existing process
    if (check_stale_pid()) {
        return -1;
    }
    
    // Write new PID file
    if (write_pid_file(PID_FILE) < 0) {
        return -1;
    }
    
    // Initialize death watch
    init_death_watch();
    
    return 0;
}
EOF

# 5. Create CLI CMakeLists.txt
echo "[5/7] Creating CLI build configuration..."
cat > src/cli/CMakeLists.txt << 'EOF'
# OBINexus CLI - Separate executable build
cmake_minimum_required(VERSION 3.10)

# CLI executable sources
set(CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/polycall_cli.c
    ${CMAKE_CURRENT_SOURCE_DIR}/monitor/pid_manager.c
)

# Create CLI executable (not part of library)
add_executable(polycall_cli ${CLI_SOURCES})

# Link with polycall libraries
if(TARGET polycall_shared)
    target_link_libraries(polycall_cli polycall_shared)
else()
    target_link_libraries(polycall_cli polycall_static)
endif()

# Additional libraries
target_link_libraries(polycall_cli 
    pthread
    ${CMAKE_DL_LIBS}
)

# Installation
install(TARGETS polycall_cli
    RUNTIME DESTINATION bin
)
EOF

# 6. Create clean Makefile for direct build
echo "[6/7] Creating direct build Makefile..."
cat > Makefile.direct << 'EOF'
# OBINexus libpolycall v2 - Direct Build (no CMake)
# Clean build without duplicates

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11 -pthread -fPIC -I./include -I./include/libpolycall
LDFLAGS = -pthread
ARFLAGS = rcs

# Build directories
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin

# Library outputs
STATIC_LIB = $(LIB_DIR)/libpolycall.a
SHARED_LIB = $(LIB_DIR)/libpolycall.so

# CLI executable
CLI_EXEC = $(BIN_DIR)/polycall

# Source files (excluding duplicates and CMake artifacts)
CORE_SOURCES = $(shell find src/core -name "*.c" -type f 2>/dev/null | \
    grep -v CMake | \
    grep -v main.c | \
    grep -v nlm_altas | \
    sort -u)

ADAPTER_SOURCES = $(shell find src/adapter -name "*.c" -type f 2>/dev/null | grep -v CMake)
SOCKET_SOURCES = $(shell find src/socket -name "*.c" -type f 2>/dev/null | grep -v CMake)
HOTWIRE_SOURCES = $(shell find src/hotwire -name "*.c" -type f 2>/dev/null | grep -v CMake)
MICRO_SOURCES = $(shell find src/micro -name "*.c" -type f 2>/dev/null | grep -v CMake)
NLM_SOURCES = $(shell find src/nlm -name "*.c" -type f 2>/dev/null | grep -v CMake)
STREAM_SOURCES = $(shell find src/stream -name "*.c" -type f 2>/dev/null | grep -v CMake)
ZERO_SOURCES = $(shell find src/zero -name "*.c" -type f 2>/dev/null | grep -v CMake)

# CLI sources
CLI_SOURCES = src/cli/polycall_cli.c src/cli/monitor/pid_manager.c

# All library sources
LIB_SOURCES = $(CORE_SOURCES) $(ADAPTER_SOURCES) $(SOCKET_SOURCES) \
              $(HOTWIRE_SOURCES) $(MICRO_SOURCES) $(NLM_SOURCES) \
              $(STREAM_SOURCES) $(ZERO_SOURCES)

# Object files
LIB_OBJECTS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(LIB_SOURCES))
CLI_OBJECTS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(CLI_SOURCES))

# Default target
all: directories $(STATIC_LIB) $(SHARED_LIB) $(CLI_EXEC)

# Create directories
directories:
	@mkdir -p $(OBJ_DIR)/core $(OBJ_DIR)/adapter $(OBJ_DIR)/socket
	@mkdir -p $(OBJ_DIR)/hotwire $(OBJ_DIR)/micro $(OBJ_DIR)/nlm
	@mkdir -p $(OBJ_DIR)/stream $(OBJ_DIR)/zero $(OBJ_DIR)/cli/monitor
	@mkdir -p $(LIB_DIR) $(BIN_DIR)

# Compile library objects
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Build static library
$(STATIC_LIB): $(LIB_OBJECTS)
	@echo "Building static library..."
	@$(AR) $(ARFLAGS) $@ $(LIB_OBJECTS)
	@echo "✓ Static library created: $@"

# Build shared library
$(SHARED_LIB): $(LIB_OBJECTS)
	@echo "Building shared library..."
	@$(CC) -shared -o $@ $(LIB_OBJECTS) $(LDFLAGS)
	@echo "✓ Shared library created: $@"

# Build CLI executable
$(CLI_EXEC): $(CLI_OBJECTS) $(STATIC_LIB)
	@echo "Building CLI executable..."
	@$(CC) $(CFLAGS) -o $@ $(CLI_OBJECTS) -L$(LIB_DIR) -lpolycall $(LDFLAGS)
	@echo "✓ CLI executable created: $@"

# Clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@find . -name "*.o" -delete
	@find . -name "CMakeC*" -delete

# Verify build
verify:
	@echo "=== Build Verification ==="
	@if [ -f "$(STATIC_LIB)" ]; then \
		echo "✓ Static library: $(STATIC_LIB)"; \
		ls -lh $(STATIC_LIB); \
	fi
	@if [ -f "$(SHARED_LIB)" ]; then \
		echo "✓ Shared library: $(SHARED_LIB)"; \
		ls -lh $(SHARED_LIB); \
	fi
	@if [ -f "$(CLI_EXEC)" ]; then \
		echo "✓ CLI executable: $(CLI_EXEC)"; \
		ls -lh $(CLI_EXEC); \
	fi

.PHONY: all clean directories verify
EOF

# 7. Update main CMakeLists.txt to exclude CLI from library
echo "[7/7] Updating root CMakeLists.txt..."
cat > CMakeLists.txt << 'EOF'
# OBINexus libpolycall v2 - Root CMake (Fixed)
cmake_minimum_required(VERSION 3.10)
project(libpolycall VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_CLI "Build CLI executable" ON)

# Thread support
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/libpolycall
)

# Collect library sources (excluding CLI and CMake artifacts)
file(GLOB_RECURSE ALL_SOURCES 
    "src/core/*.c"
    "src/adapter/*.c"
    "src/socket/*.c"
    "src/hotwire/*.c"
    "src/micro/*.c"
    "src/nlm/*.c"
    "src/stream/*.c"
    "src/zero/*.c"
)

# Filter out unwanted files
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*CMake.*")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main\\.c")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*nlm_altas.*")  # typo version

# Remove duplicates
list(REMOVE_DUPLICATES ALL_SOURCES)

# Build libraries
if(BUILD_STATIC_LIBS)
    add_library(polycall_static STATIC ${ALL_SOURCES})
    target_link_libraries(polycall_static Threads::Threads)
    set_target_properties(polycall_static PROPERTIES
        OUTPUT_NAME "polycall"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

if(BUILD_SHARED_LIBS)
    add_library(polycall_shared SHARED ${ALL_SOURCES})
    target_link_libraries(polycall_shared Threads::Threads)
    set_target_properties(polycall_shared PROPERTIES
        OUTPUT_NAME "polycall"
        VERSION ${PROJECT_VERSION}
        SOVERSION 2
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

# Build CLI separately
if(BUILD_CLI AND EXISTS "${CMAKE_SOURCE_DIR}/src/cli")
    add_subdirectory(src/cli)
endif()

# Installation
install(DIRECTORY include/ DESTINATION include)
if(TARGET polycall_static)
    install(TARGETS polycall_static DESTINATION lib)
endif()
if(TARGET polycall_shared)
    install(TARGETS polycall_shared DESTINATION lib)
endif()
EOF

echo "=== Build Fix Complete ==="
echo ""
echo "Changes made:"
echo "  ✓ Removed all CMake test artifacts"
echo "  ✓ Moved main.c to src/cli/polycall_cli.c"
echo "  ✓ Created PID manager with death watch"
echo "  ✓ Removed duplicate source files"
echo "  ✓ Created separate CLI build configuration"
echo "  ✓ Updated CMakeLists.txt files"
echo ""
echo "Build with:"
echo "  make -f Makefile.direct     # Direct build (recommended)"
echo "  cmake . && make             # CMake build"
echo ""
echo "The CLI will:"
echo "  • Log PID to /var/run/polycall.pid or /tmp/polycall.pid"
echo "  • Implement death watch for clean shutdown"
echo "  • Check for stale processes on startup"

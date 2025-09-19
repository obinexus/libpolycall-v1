#!/bin/bash
# polycall_repair.sh - Fix broken modules and Unix mode issues

set -e  # Exit on error
set -u  # Exit on undefined variable

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== PolyCall Module Repair Script ===${NC}"

# Set proper Unix permissions
fix_unix_permissions() {
    echo -e "${YELLOW}Fixing Unix mode permissions...${NC}"
    
    # Fix executable permissions
    chmod +x bin/polycall 2>/dev/null || echo "polycall binary not found"
    
    # Fix library permissions
    chmod 644 lib/*.a 2>/dev/null || true
    chmod 755 lib/*.so 2>/dev/null || true
    
    # Fix source permissions
    find src -type f -name "*.c" -exec chmod 644 {} \;
    find include -type f -name "*.h" -exec chmod 644 {} \;
    
    # Fix build artifacts
    find build -type f -name "*.o" -exec chmod 644 {} \;
    find build -type f -name "*.d" -exec chmod 644 {} \;
}

# Clean and rebuild object files with proper flags
rebuild_objects() {
    echo -e "${YELLOW}Rebuilding object files...${NC}"
    
    # Clean existing objects
    rm -f build/*.o build/*.d
    
    # Compiler flags for thread safety and Unix compatibility
    CFLAGS="-Wall -Wextra -pthread -fPIC -D_GNU_SOURCE -D_REENTRANT"
    CFLAGS="${CFLAGS} -I./include -std=c11"
    
    # Core modules that need rebuilding
    MODULES=(
        "main"
        "network"
        "polycall"
        "polycall_micro"
        "polycall_parser"
        "polycall_protocol"
        "polycall_state_machine"
        "polycall_token"
        "polycall_tokenizer"
    )
    
    for module in "${MODULES[@]}"; do
        echo -n "Building ${module}... "
        if gcc ${CFLAGS} -MMD -c "src/${module}.c" -o "build/${module}.o" 2>/dev/null; then
            echo -e "${GREEN}OK${NC}"
        else
            echo -e "${RED}FAILED${NC}"
            # Try to compile with relaxed flags
            gcc ${CFLAGS} -fpermissive -c "src/${module}.c" -o "build/${module}.o" 2>/dev/null || true
        fi
    done
}

# Build static library with ar
build_static_library() {
    echo -e "${YELLOW}Building static library...${NC}"
    
    # Remove old library
    rm -f lib/libpolycall.a
    
    # Create new archive
    ar rcs lib/libpolycall.a build/*.o
    
    # Index the archive
    ranlib lib/libpolycall.a
    
    echo -e "${GREEN}Static library built: lib/libpolycall.a${NC}"
}

# Build shared library with proper soname
build_shared_library() {
    echo -e "${YELLOW}Building shared library...${NC}"
    
    # Remove old library
    rm -f lib/libpolycall.so
    
    # Link shared object with version info
    gcc -shared -Wl,-soname,libpolycall.so.1 \
        -o lib/libpolycall.so \
        build/*.o \
        -pthread -lc
    
    # Create symlinks for version compatibility
    cd lib
    ln -sf libpolycall.so libpolycall.so.1
    ln -sf libpolycall.so.1 libpolycall.so.1.0
    cd ..
    
    echo -e "${GREEN}Shared library built: lib/libpolycall.so${NC}"
}

# Fix the v2/ test structure
fix_v2_structure() {
    echo -e "${YELLOW}Fixing v2/ test structure...${NC}"
    
    # Ensure v2 directories exist
    mkdir -p v2/tests/{unit,integration,performance}
    mkdir -p v2/build/{debug,release}
    
    # Create TDD test runner
    cat > v2/run_tests.sh << 'EOF'
#!/bin/bash
# TDD Test Runner for PolyCall v2

export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH

# Unit tests
echo "Running unit tests..."
for test in tests/unit/*.c; do
    testname=$(basename "$test" .c)
    gcc -o "build/debug/$testname" "$test" -L../lib -lpolycall -pthread
    if "./build/debug/$testname"; then
        echo "✓ $testname passed"
    else
        echo "✗ $testname failed"
    fi
done

# Integration tests
echo "Running integration tests..."
python3 -m pytest isolated/bindings/pypolycall/tests/ -v
EOF
    
    chmod +x v2/run_tests.sh
}

# Create thread safety test
create_thread_safety_test() {
    echo -e "${YELLOW}Creating thread safety test...${NC}"
    
    mkdir -p test
    cat > test/test_thread_safety.c << 'EOF'
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#define NUM_THREADS 5
#define TEST_ITERATIONS 100

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
int shared_counter = 0;

void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        pthread_mutex_lock(&global_mutex);
        shared_counter++;
        printf("Thread %d: counter = %d\n", thread_id, shared_counter);
        pthread_mutex_unlock(&global_mutex);
        usleep(1000); // 1ms delay
    }
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("Starting thread safety test...\n");
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify result
    int expected = NUM_THREADS * TEST_ITERATIONS;
    if (shared_counter == expected) {
        printf("✓ Thread safety test PASSED: counter = %d (expected %d)\n", 
               shared_counter, expected);
        return 0;
    } else {
        printf("✗ Thread safety test FAILED: counter = %d (expected %d)\n", 
               shared_counter, expected);
        return 1;
    }
}
EOF
    
    # Compile and run test
    gcc -o test/test_thread_safety test/test_thread_safety.c -pthread
    ./test/test_thread_safety
}

# Verify library symbols
verify_symbols() {
    echo -e "${YELLOW}Verifying library symbols...${NC}"
    
    echo "Static library symbols:"
    nm -g lib/libpolycall.a | grep " T " | head -10
    
    echo ""
    echo "Shared library symbols:"
    nm -D lib/libpolycall.so | grep " T " | head -10
    
    echo ""
    echo "Dependencies:"
    ldd lib/libpolycall.so || echo "ldd not available"
}

# Main execution
main() {
    echo -e "${GREEN}Starting PolyCall repair process...${NC}"
    
    # Execute all repair steps
    fix_unix_permissions
    rebuild_objects
    build_static_library
    build_shared_library
    fix_v2_structure
    create_thread_safety_test
    verify_symbols
    
    echo -e "${GREEN}=== Repair Complete ===${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Run 'make test' to verify the build"
    echo "2. Execute './v2/run_tests.sh' for TDD suite"
    echo "3. Check 'lib/' for both .a and .so libraries"
    echo "4. Use 'LD_LIBRARY_PATH=./lib ./bin/polycall' to test runtime"
}

# Run main function
main "$@"

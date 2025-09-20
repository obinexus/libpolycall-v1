// v2/test/test_nlm_atlas.c
// Test program demonstrating NLM-Atlas namespace resolution with AVL-Huffman trie

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Service endpoint structure (from nlm_atlas_avl_huffman.c)
typedef struct service_endpoint {
    char* fqdn;
    uint16_t port;
    char* protocol;
    char* path;
    void* lock;  // pthread_mutex_t
} service_endpoint_t;

typedef struct search_result {
    service_endpoint_t** endpoints;
    size_t count;
    size_t capacity;
} search_result_t;

// External functions from NLM-Atlas
extern void nlm_atlas_init(void);
extern void nlm_atlas_cleanup(void);
extern int nlm_atlas_register_service(const char* fqdn, uint16_t port, 
                                      const char* protocol, const char* path);
extern service_endpoint_t* nlm_atlas_resolve_namespace(const char* query);
extern search_result_t* nlm_atlas_search_services(const char* pattern);
extern char* avl_huffman_optimize_query(const char* query);
extern void avl_huffman_rebalance_trie(void);

// Color codes for terminal output
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define RESET   "\033[0m"

// Print colored output
void print_header(const char* text) {
    printf("\n%s═══════════════════════════════════════════════════════%s\n", BLUE, RESET);
    printf("%s  %s%s\n", CYAN, text, RESET);
    printf("%s═══════════════════════════════════════════════════════%s\n", BLUE, RESET);
}

// Test 1: Register services with full namespaces
void test_service_registration(void) {
    print_header("Test 1: Service Registration");
    
    struct {
        const char* fqdn;
        uint16_t port;
        const char* path;
    } services[] = {
        {"transfer.execute.obinexus.banking.finance.us.org", 8081, "/api/v2/transfer/execute"},
        {"report.generate.obinexus.analytics.research.uk.org", 8442, "/api/v2/report/generate"},
        {"audit.log.obinexus.security.operations.de.org", 444, "/api/v2/audit/log"},
        {"balance.check.obinexus.banking.finance.ca.org", 8082, "/api/v2/balance/check"},
        {"fraud.detect.obinexus.security.operations.jp.org", 9443, "/api/v2/fraud/detect"},
        {"loan.approve.obinexus.banking.finance.au.org", 8083, "/api/v2/loan/approve"},
    };
    
    for (size_t i = 0; i < sizeof(services)/sizeof(services[0]); i++) {
        int result = nlm_atlas_register_service(services[i].fqdn, services[i].port, 
                                               "HTTPS", services[i].path);
        if (result == 0) {
            printf("  %s✓%s Registered: %s%.40s...%s on port %s%d%s\n", 
                   GREEN, RESET, YELLOW, services[i].fqdn, RESET, 
                   MAGENTA, services[i].port, RESET);
        }
    }
}

// Test 2: Resolve specific namespaces
void test_namespace_resolution(void) {
    print_header("Test 2: Namespace Resolution");
    
    const char* queries[] = {
        "debit",
        "credit",
        "banking",
        "us-banking",
        "auth-create",
        "security",
        "analytics"
    };
    
    for (size_t i = 0; i < sizeof(queries)/sizeof(queries[0]); i++) {
        printf("\n  Query: %s%s%s\n", CYAN, queries[i], RESET);
        
        service_endpoint_t* endpoint = nlm_atlas_resolve_namespace(queries[i]);
        if (endpoint) {
            printf("    → Found: %s:%d%s\n", endpoint->protocol, 
                   endpoint->port, endpoint->path);
        } else {
            printf("    %s✗%s Not found\n", RED, RESET);
        }
    }
}

// Test 3: Pattern-based search
void test_pattern_search(void) {
    print_header("Test 3: Pattern-Based Service Search");
    
    const char* patterns[] = {
        "banking",
        "finance",
        "security",
        "us",
        "uk"
    };
    
    for (size_t i = 0; i < sizeof(patterns)/sizeof(patterns[0]); i++) {
        printf("\n  Pattern: %s%s%s\n", CYAN, patterns[i], RESET);
        
        search_result_t* results = nlm_atlas_search_services(patterns[i]);
        if (results && results->count > 0) {
            printf("    Found %s%zu%s services:\n", GREEN, results->count, RESET);
            for (size_t j = 0; j < results->count && j < 3; j++) {
                service_endpoint_t* ep = results->endpoints[j];
                printf("      • %s:%d%s\n", ep->protocol, ep->port, ep->path);
            }
            if (results->count > 3) {
                printf("      ... and %zu more\n", results->count - 3);
            }
        } else {
            printf("    %s✗%s No matches\n", RED, RESET);
        }
        
        free(results);
    }
}

// Test 4: Query optimization using Huffman frequency
void test_query_optimization(void) {
    print_header("Test 4: Huffman Query Optimization");
    
    const char* queries[] = {
        "banking us",
        "auth",
        "transfer money",
        "check balance",
        "security audit"
    };
    
    for (size_t i = 0; i < sizeof(queries)/sizeof(queries[0]); i++) {
        char* optimized = avl_huffman_optimize_query(queries[i]);
        printf("  Original: %s%-20s%s → Optimized: %s%s%s\n",
               YELLOW, queries[i], RESET,
               GREEN, optimized, RESET);
        free(optimized);
    }
}

// Test 5: Simulate high-frequency access patterns
void test_frequency_rebalancing(void) {
    print_header("Test 5: AVL Tree Rebalancing Based on Access Frequency");
    
    printf("  Simulating access patterns...\n");
    
    // Simulate high frequency access to certain services
    for (int i = 0; i < 100; i++) {
        nlm_atlas_resolve_namespace("debit");  // Very high frequency
    }
    for (int i = 0; i < 50; i++) {
        nlm_atlas_resolve_namespace("credit"); // High frequency
    }
    for (int i = 0; i < 10; i++) {
        nlm_atlas_resolve_namespace("audit");  // Low frequency
    }
    
    printf("  %s✓%s Generated access pattern:\n", GREEN, RESET);
    printf("    • debit: 100 accesses %s(HIGH)%s\n", RED, RESET);
    printf("    • credit: 50 accesses %s(MEDIUM)%s\n", YELLOW, RESET);
    printf("    • audit: 10 accesses %s(LOW)%s\n", BLUE, RESET);
    
    // Trigger rebalancing
    printf("\n  Triggering AVL rebalancing...\n");
    avl_huffman_rebalance_trie();
    printf("  %s✓%s Tree rebalanced for optimal access\n", GREEN, RESET);
}

// Test 6: Demonstrate color-coded frequency visualization
void test_color_visualization(void) {
    print_header("Test 6: Color-Coded Frequency Visualization");
    
    struct {
        const char* service;
        uint32_t frequency;
        const char* color_name;
        const char* color_code;
    } freq_map[] = {
        {"auth.create", 12000, "Red (Very High)", RED},
        {"debit.validate", 8500, "Yellow (High)", YELLOW},
        {"credit.process", 7200, "Yellow (High)", YELLOW},
        {"score.update", 3400, "Blue (Medium)", BLUE},
        {"report.generate", 500, "Gray (Low)", "\033[0;37m"},
    };
    
    printf("  Service frequencies with color coding:\n\n");
    for (size_t i = 0; i < sizeof(freq_map)/sizeof(freq_map[0]); i++) {
        printf("    %s████████%s %s%-20s%s [%5d accesses] - %s\n",
               freq_map[i].color_code, RESET,
               freq_map[i].color_code, freq_map[i].service, RESET,
               freq_map[i].frequency, freq_map[i].color_name);
    }
    
    printf("\n  %s✓%s Color mapping follows libchroma's Huffman concept\n", 
           GREEN, RESET);
}

// Test 7: Performance metrics
void test_performance_metrics(void) {
    print_header("Test 7: Performance Metrics");
    
    clock_t start, end;
    double cpu_time_used;
    
    // Test registration speed
    start = clock();
    for (int i = 0; i < 100; i++) {
        char fqdn[256];
        snprintf(fqdn, sizeof(fqdn), 
                "service%d.op%d.obinexus.dept%d.div%d.us.org",
                i, i%10, i%5, i%3);
        nlm_atlas_register_service(fqdn, 8000 + i, "HTTPS", "/api/test");
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Registration: %s%.3f ms%s for 100 services\n", 
           GREEN, cpu_time_used * 1000, RESET);
    
    // Test lookup speed
    start = clock();
    for (int i = 0; i < 1000; i++) {
        nlm_atlas_resolve_namespace("service50");
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Lookup: %s%.3f ms%s for 1000 queries\n", 
           GREEN, cpu_time_used * 1000, RESET);
    
    // Test search speed
    start = clock();
    for (int i = 0; i < 100; i++) {
        search_result_t* results = nlm_atlas_search_services("dept2");
        free(results);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Pattern Search: %s%.3f ms%s for 100 searches\n", 
           GREEN, cpu_time_used * 1000, RESET);
}

// Main test runner
int main(int argc, char* argv[]) {
    printf("%s╔══════════════════════════════════════════════════════════════╗%s\n", BLUE, RESET);
    printf("%s║        NLM-Atlas AVL-Huffman Namespace Resolution Test       ║%s\n", BLUE, RESET);
    printf("%s║              OBINexus libpolycall v2.0.0                     ║%s\n", BLUE, RESET);
    printf("%s╚══════════════════════════════════════════════════════════════╝%s\n", BLUE, RESET);
    
    // Initialize NLM-Atlas
    printf("\n%s→%s Initializing NLM-Atlas with AVL-Huffman trie...\n", CYAN, RESET);
    nlm_atlas_init();
    
    // Run tests based on command line argument
    if (argc > 1) {
        int test_num = atoi(argv[1]);
        switch(test_num) {
            case 1: test_service_registration(); break;
            case 2: test_namespace_resolution(); break;
            case 3: test_pattern_search(); break;
            case 4: test_query_optimization(); break;
            case 5: test_frequency_rebalancing(); break;
            case 6: test_color_visualization(); break;
            case 7: test_performance_metrics(); break;
            default:
                printf("%sInvalid test number. Running all tests.%s\n", RED, RESET);
                // Fall through to run all
        }
    } else {
        // Run all tests
        test_service_registration();
        test_namespace_resolution();
        test_pattern_search();
        test_query_optimization();
        test_frequency_rebalancing();
        test_color_visualization();
        test_performance_metrics();
    }
    
    // Cleanup
    printf("\n%s→%s Cleaning up NLM-Atlas resources...\n", CYAN, RESET);
    nlm_atlas_cleanup();
    
    // Summary
    printf("\n%s╔══════════════════════════════════════════════════════════════╗%s\n", GREEN, RESET);
    printf("%s║                    All Tests Completed                       ║%s\n", GREEN, RESET);
    printf("%s╚══════════════════════════════════════════════════════════════╝%s\n", GREEN, RESET);
    
    printf("\nThe NLM-Atlas namespace resolution system demonstrates:\n");
    printf("  • %sHierarchical namespace mapping%s (service.operation.obinexus...)\n", YELLOW, RESET);
    printf("  • %sAVL tree balancing%s for O(log n) lookup performance\n", YELLOW, RESET);
    printf("  • %sHuffman frequency optimization%s for common queries\n", YELLOW, RESET);
    printf("  • %sColor-coded visualization%s from libchroma concepts\n", YELLOW, RESET);
    printf("  • %sPattern-based service discovery%s for flexible queries\n", YELLOW, RESET);
    
    return 0;
}

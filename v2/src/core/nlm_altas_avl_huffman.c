// v2/src/core/nlm_atlas_avl_huffman.c
// NLM-Atlas namespace resolution with AVL-Huffman trie optimization

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

// AVL-Huffman node structure for namespace trie
typedef struct avl_huffman_node {
    char* key;                          // Namespace component
    uint32_t frequency;                 // Access frequency for Huffman
    uint32_t color;                     // Color coding (from libchroma concept)
    int height;                         // AVL tree height
    struct avl_huffman_node* left;
    struct avl_huffman_node* right;
    void* service_data;                 // Pointer to service endpoint
} avl_huffman_node_t;

// Service endpoint structure
typedef struct service_endpoint {
    char* fqdn;                         // Full namespace
    uint16_t port;
    char* protocol;
    char* path;
    pthread_mutex_t lock;
} service_endpoint_t;

// Namespace components
typedef struct namespace_parts {
    char* service;
    char* operation;
    char* department;
    char* division;
    char* country;
} namespace_parts_t;

// Global trie root
static avl_huffman_node_t* g_namespace_trie = NULL;
static pthread_rwlock_t g_trie_lock = PTHREAD_RWLOCK_INITIALIZER;

// Color mapping based on frequency (from libchroma concept)
static uint32_t frequency_to_color(uint32_t frequency) {
    if (frequency > 10000) return 0xFF4444;  // Red: high frequency
    if (frequency > 5000)  return 0x44FF44;  // Green: medium
    if (frequency > 1000)  return 0x4444FF;  // Blue: low
    return 0x808080;                         // Gray: rare
}

// Get AVL node height
static int avl_height(avl_huffman_node_t* node) {
    return node ? node->height : 0;
}

// Get balance factor
static int avl_balance_factor(avl_huffman_node_t* node) {
    return node ? avl_height(node->left) - avl_height(node->right) : 0;
}

// Update node height and color
static void avl_update_node(avl_huffman_node_t* node) {
    if (!node) return;
    
    int left_h = avl_height(node->left);
    int right_h = avl_height(node->right);
    node->height = 1 + (left_h > right_h ? left_h : right_h);
    node->color = frequency_to_color(node->frequency);
}

// AVL right rotation (for left-heavy subtree)
static avl_huffman_node_t* avl_rotate_right(avl_huffman_node_t* y) {
    avl_huffman_node_t* x = y->left;
    avl_huffman_node_t* T2 = x->right;
    
    // Perform rotation
    x->right = y;
    y->left = T2;
    
    // Update heights
    avl_update_node(y);
    avl_update_node(x);
    
    // Color averaging (from libchroma concept)
    x->color = ((x->color & 0xFF0000) + (y->color & 0xFF0000)) / 2 |
               ((x->color & 0x00FF00) + (y->color & 0x00FF00)) / 2 |
               ((x->color & 0x0000FF) + (y->color & 0x0000FF)) / 2;
    
    return x;
}

// AVL left rotation (for right-heavy subtree)
static avl_huffman_node_t* avl_rotate_left(avl_huffman_node_t* x) {
    avl_huffman_node_t* y = x->right;
    avl_huffman_node_t* T2 = y->left;
    
    // Perform rotation
    y->left = x;
    x->right = T2;
    
    // Update heights
    avl_update_node(x);
    avl_update_node(y);
    
    // Color averaging
    y->color = ((x->color & 0xFF0000) + (y->color & 0xFF0000)) / 2 |
               ((x->color & 0x00FF00) + (y->color & 0x00FF00)) / 2 |
               ((x->color & 0x0000FF) + (y->color & 0x0000FF)) / 2;
    
    return y;
}

// Insert node with AVL balancing
static avl_huffman_node_t* avl_insert(avl_huffman_node_t* node, 
                                      const char* key, 
                                      service_endpoint_t* endpoint) {
    // Standard BST insertion
    if (!node) {
        avl_huffman_node_t* new_node = malloc(sizeof(avl_huffman_node_t));
        new_node->key = strdup(key);
        new_node->frequency = 1;
        new_node->color = frequency_to_color(1);
        new_node->height = 1;
        new_node->left = new_node->right = NULL;
        new_node->service_data = endpoint;
        return new_node;
    }
    
    int cmp = strcmp(key, node->key);
    if (cmp < 0) {
        node->left = avl_insert(node->left, key, endpoint);
    } else if (cmp > 0) {
        node->right = avl_insert(node->right, key, endpoint);
    } else {
        // Key exists, increase frequency
        node->frequency++;
        node->color = frequency_to_color(node->frequency);
        return node;
    }
    
    // Update height
    avl_update_node(node);
    
    // Get balance factor
    int balance = avl_balance_factor(node);
    
    // Left Heavy
    if (balance > 1) {
        if (strcmp(key, node->left->key) < 0) {
            // Left-Left case
            return avl_rotate_right(node);
        } else {
            // Left-Right case
            node->left = avl_rotate_left(node->left);
            return avl_rotate_right(node);
        }
    }
    
    // Right Heavy
    if (balance < -1) {
        if (strcmp(key, node->right->key) > 0) {
            // Right-Right case
            return avl_rotate_left(node);
        } else {
            // Right-Left case
            node->right = avl_rotate_right(node->right);
            return avl_rotate_left(node);
        }
    }
    
    return node;
}

// Parse namespace into components
namespace_parts_t* parse_namespace(const char* fqdn) {
    namespace_parts_t* parts = calloc(1, sizeof(namespace_parts_t));
    char* temp = strdup(fqdn);
    char* token;
    char* saveptr;
    int index = 0;
    
    // Parse: service.operation.obinexus.department.division.country.org
    token = strtok_r(temp, ".", &saveptr);
    while (token != NULL && index < 7) {
        switch(index) {
            case 0: parts->service = strdup(token); break;
            case 1: parts->operation = strdup(token); break;
            // Skip "obinexus" at index 2
            case 3: parts->department = strdup(token); break;
            case 4: parts->division = strdup(token); break;
            case 5: parts->country = strdup(token); break;
            // Skip "org" at index 6
        }
        token = strtok_r(NULL, ".", &saveptr);
        index++;
    }
    
    free(temp);
    return parts;
}

// Build search key from namespace parts
static char* build_search_key(namespace_parts_t* parts) {
    char key[256];
    snprintf(key, sizeof(key), "%s-%s-%s-%s", 
             parts->service ? parts->service : "*",
             parts->operation ? parts->operation : "*",
             parts->country ? parts->country : "*",
             parts->department ? parts->department : "*");
    return strdup(key);
}

// Search with pattern matching
static avl_huffman_node_t* search_pattern(avl_huffman_node_t* node, 
                                          const char* pattern) {
    if (!node) return NULL;
    
    // Check if pattern matches current node
    if (strstr(node->key, pattern) != NULL) {
        node->frequency++;  // Increase frequency on access
        node->color = frequency_to_color(node->frequency);
        return node;
    }
    
    // Search both subtrees
    avl_huffman_node_t* left_result = search_pattern(node->left, pattern);
    if (left_result) return left_result;
    
    return search_pattern(node->right, pattern);
}

// Public API: Register a service in the namespace
int nlm_atlas_register_service(const char* fqdn, uint16_t port, 
                               const char* protocol, const char* path) {
    if (!fqdn || !protocol || !path) return -1;
    
    // Create endpoint
    service_endpoint_t* endpoint = malloc(sizeof(service_endpoint_t));
    endpoint->fqdn = strdup(fqdn);
    endpoint->port = port;
    endpoint->protocol = strdup(protocol);
    endpoint->path = strdup(path);
    pthread_mutex_init(&endpoint->lock, NULL);
    
    // Parse namespace
    namespace_parts_t* parts = parse_namespace(fqdn);
    char* key = build_search_key(parts);
    
    // Insert into trie with write lock
    pthread_rwlock_wrlock(&g_trie_lock);
    g_namespace_trie = avl_insert(g_namespace_trie, key, endpoint);
    pthread_rwlock_unlock(&g_trie_lock);
    
    // Cleanup
    free(key);
    free(parts->service);
    free(parts->operation);
    free(parts->department);
    free(parts->division);
    free(parts->country);
    free(parts);
    
    printf("[NLM-Atlas] Registered service: %s on port %d\n", fqdn, port);
    return 0;
}

// Public API: Resolve namespace to service endpoint
service_endpoint_t* nlm_atlas_resolve_namespace(const char* query) {
    if (!query) return NULL;
    
    pthread_rwlock_rdlock(&g_trie_lock);
    
    // Search for pattern in trie
    avl_huffman_node_t* node = search_pattern(g_namespace_trie, query);
    service_endpoint_t* endpoint = node ? node->service_data : NULL;
    
    pthread_rwlock_unlock(&g_trie_lock);
    
    if (endpoint) {
        printf("[NLM-Atlas] Resolved: %s -> %s:%d%s\n", 
               query, endpoint->protocol, endpoint->port, endpoint->path);
    }
    
    return endpoint;
}

// Public API: Search services with pattern
typedef struct search_result {
    service_endpoint_t** endpoints;
    size_t count;
    size_t capacity;
} search_result_t;

static void collect_matching_services(avl_huffman_node_t* node, 
                                      const char* pattern,
                                      search_result_t* results) {
    if (!node) return;
    
    // In-order traversal for sorted results
    collect_matching_services(node->left, pattern, results);
    
    if (strstr(node->key, pattern) != NULL && node->service_data) {
        if (results->count >= results->capacity) {
            results->capacity *= 2;
            results->endpoints = realloc(results->endpoints, 
                                        results->capacity * sizeof(service_endpoint_t*));
        }
        results->endpoints[results->count++] = node->service_data;
    }
    
    collect_matching_services(node->right, pattern, results);
}

search_result_t* nlm_atlas_search_services(const char* pattern) {
    search_result_t* results = malloc(sizeof(search_result_t));
    results->endpoints = malloc(10 * sizeof(service_endpoint_t*));
    results->count = 0;
    results->capacity = 10;
    
    pthread_rwlock_rdlock(&g_trie_lock);
    collect_matching_services(g_namespace_trie, pattern, results);
    pthread_rwlock_unlock(&g_trie_lock);
    
    printf("[NLM-Atlas] Found %zu services matching '%s'\n", 
           results->count, pattern);
    return results;
}

// Public API: Optimize query using Huffman frequency
char* avl_huffman_optimize_query(const char* query) {
    // Build optimized query based on frequency patterns
    char optimized[256];
    
    // Check common patterns
    if (strstr(query, "banking") && strstr(query, "us")) {
        // High frequency pattern
        snprintf(optimized, sizeof(optimized), "debit-*-us-banking");
    } else if (strstr(query, "auth")) {
        // Very high frequency
        snprintf(optimized, sizeof(optimized), "auth-create-*-security");
    } else {
        // Default pattern
        snprintf(optimized, sizeof(optimized), "*-%s-*-*", query);
    }
    
    return strdup(optimized);
}

// Public API: Rebalance the trie based on access patterns
static avl_huffman_node_t* rebalance_by_frequency(avl_huffman_node_t* node) {
    if (!node) return NULL;
    
    // Recursively rebalance children
    node->left = rebalance_by_frequency(node->left);
    node->right = rebalance_by_frequency(node->right);
    
    // Check if rebalancing needed based on frequency difference
    uint32_t left_freq = node->left ? node->left->frequency : 0;
    uint32_t right_freq = node->right ? node->right->frequency : 0;
    
    if (left_freq > right_freq * 2 && node->right) {
        // Left child accessed much more, rotate right
        return avl_rotate_right(node);
    } else if (right_freq > left_freq * 2 && node->left) {
        // Right child accessed much more, rotate left
        return avl_rotate_left(node);
    }
    
    return node;
}

void avl_huffman_rebalance_trie(void) {
    pthread_rwlock_wrlock(&g_trie_lock);
    g_namespace_trie = rebalance_by_frequency(g_namespace_trie);
    pthread_rwlock_unlock(&g_trie_lock);
    printf("[NLM-Atlas] Trie rebalanced based on access frequency\n");
}

// Initialize NLM-Atlas with default services
void nlm_atlas_init(void) {
    // Register default banking services
    nlm_atlas_register_service("debit.validate.obinexus.banking.finance.us.org", 
                               8080, "HTTPS", "/api/v2/debit/validate");
    
    nlm_atlas_register_service("credit.process.obinexus.banking.finance.us.org", 
                               9056, "HTTPS", "/api/v2/credit/process");
    
    nlm_atlas_register_service("score.update.obinexus.analytics.research.uk.org", 
                               8443, "HTTPS", "/api/v2/score/update");
    
    nlm_atlas_register_service("auth.create.obinexus.security.operations.de.org", 
                               443, "HTTPS", "/api/v2/auth/create");
    
    printf("[NLM-Atlas] Initialized with AVL-Huffman trie\n");
}

// Cleanup
void nlm_atlas_cleanup(void) {
    // Would implement full tree cleanup here
    pthread_rwlock_destroy(&g_trie_lock);
}

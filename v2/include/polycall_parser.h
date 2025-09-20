#ifndef POLYCALL_PARSER_H
#define POLYCALL_PARSER_H

#include "polycall_token.h"
#include "polycall_tokenizer.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// AST node types
typedef enum {
    AST_NONE          = 0x00,
    AST_PROGRAM       = 0x01,
    AST_FUNCTION      = 0x02,
    AST_VARIABLE      = 0x04,
    AST_EXPRESSION    = 0x08,
    AST_STATEMENT     = 0x10,
    AST_BLOCK         = 0x20,
    AST_CONTROL_FLOW  = 0x40,
    AST_ERROR         = 0x80
} PolycallASTType;

// Node attributes
typedef enum {
    AST_ATTR_NONE     = 0x00,
    AST_ATTR_PUBLIC   = 0x01,
    AST_ATTR_PRIVATE  = 0x02,
    AST_ATTR_STATIC   = 0x04,
    AST_ATTR_CONST    = 0x08,
    AST_ATTR_VOLATILE = 0x10,
    AST_ATTR_EXPORTED = 0x20
} PolycallASTAttributes;

// Forward declaration from token.h
typedef union PolycallValue PolycallValue;

// AST node structure
typedef struct PolycallASTNode {
    PolycallASTType type;
    PolycallASTAttributes attrs;
    PolycallValue value;
    uint32_t line;
    uint32_t column;
    uint32_t child_count;
    struct PolycallASTNode** children;
    struct PolycallASTNode* parent;
} PolycallASTNode;

// AST structure
typedef struct {
    PolycallASTNode* root;
    PolycallASTNode** nodes;
    uint32_t node_count;
    uint32_t capacity;
    uint32_t error_count;
} PolycallAST;

// Parser configuration
typedef struct {
    size_t max_depth;
    size_t max_nodes;
    uint32_t optimization_level;
    bool strict_mode;
    void* user_data;
} PolycallParserConfig;

// Function pointer types
typedef PolycallASTNode* (*ASTTransform)(const PolycallASTNode*);
typedef bool (*ASTPredicate)(const PolycallASTNode*);
typedef void (*ASTVisitor)(PolycallASTNode*, void*);

// Transform chain
typedef struct {
    ASTTransform* transforms;
    uint32_t count;
} PolycallASTTransforms;

// Parser context
typedef struct {
    struct PolycallTokenizer* tokenizer;
    PolycallAST* ast;
    const PolycallParserConfig* config;
    struct {
        char* message;
        uint32_t line;
        uint32_t column;
    } error;
} PolycallParser;

// Function declarations
PolycallParser* polycall_parser_create(const PolycallParserConfig* config);
void polycall_parser_destroy(PolycallParser* parser);
PolycallAST* polycall_parser_parse_file(PolycallParser* parser, const char* filename);
PolycallAST* polycall_parser_parse_string(PolycallParser* parser, const char* input, size_t length);
PolycallASTNode* polycall_ast_create_node(PolycallASTType type, const PolycallValue* value);
void polycall_ast_destroy_node(PolycallASTNode* node);
bool polycall_ast_add_child(PolycallASTNode* parent, PolycallASTNode* child);
PolycallASTNode* polycall_ast_map(const PolycallASTNode* node, ASTTransform transform);
PolycallAST* polycall_ast_filter(const PolycallAST* ast, ASTPredicate predicate);
void polycall_ast_visit(PolycallASTNode* node, ASTVisitor visitor, void* user_data);
PolycallASTTransforms* polycall_ast_create_transforms(ASTTransform* transforms, uint32_t count);
void polycall_ast_destroy_transforms(PolycallASTTransforms* transforms);
PolycallAST* polycall_ast_apply_transforms(const PolycallAST* ast, const PolycallASTTransforms* transforms);
bool polycall_ast_validate(const PolycallAST* ast);
const char* polycall_parser_get_error(const PolycallParser* parser);
PolycallASTNode* polycall_ast_find_node(const PolycallAST* ast, PolycallASTType type);
PolycallASTNode** polycall_ast_find_nodes(const PolycallAST* ast, PolycallASTType type, uint32_t* count);
PolycallAST* polycall_ast_optimize(const PolycallAST* ast, uint32_t level);
void polycall_ast_set_attributes(PolycallASTNode* node, PolycallASTAttributes attrs);
PolycallASTAttributes polycall_ast_get_attributes(const PolycallASTNode* node);
void polycall_ast_set_location(PolycallASTNode* node, uint32_t line, uint32_t column);
void polycall_ast_get_location(const PolycallASTNode* node, uint32_t* line, uint32_t* column);

#endif // POLYCALL_PARSER_H

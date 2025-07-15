#ifndef _AST_H
#define _AST_H

#include <stddef.h>

#include "string_pool.h"
#include "tokenizer.h"

// // NOTE: THIS IS FULLY RANDOM :)
// typedef enum {
//     AST_INVALID,
//     AST_FUNC_DECL,
//     AST_FUNC_DEF,
//     AST_VAR_DECL,
//     AST_VAR_DEF,
//     AST_EXPR,
//     AST_OP,
//     AST_BRANCH,
//     AST_LOOP,
// } branch_t;

// typedef struct {
//     branch_t type;
//     size_t size;
//     char data[];
// } Branch;

typedef enum {
    AST_ROOT,
    AST_ASSIGN,
    AST_TYPE,
    AST_DECL,
    AST_EXPR,
    AST_RETURN,
    AST_IDENT,
    AST_LIT_INT,
    AST_OP_ADD,
    AST_OP_MUL,
} ast_node_t;

// // NOTE: this is not definitive nor the full/correct syntax :)
// typedef enum {
//     AST_INVALID = 0,
//     // AST_LIT  := integer | string
//     AST_LIT,
//     // AST_OP   := AST_VAL_EXPR [op AST_VAL_EXPR ]*
//     AST_OP,
//     // VAL_TYPE := PRIMITIVE
//     //           | OP
//     //           | BLOCK
//     //           | "func" "(" ( | "void" | [PARAM ["," PARAM]*]) ")" "->"
//     //             (PRIMITIVE | "void")
//     AST_TYPE,
//     // VAL_EXPR := VAL_EXPR
//     //           | DECL
//     //           | DEF
//     AST_EXPR,
//     // VAL_BLOCK := "{" [EXPR]* "}"
//     AST_BLOCK,
//     // VAL_EXPR := LIT
//     //           | OP
//     //           | unari_op VAL_EXPR
//     AST_VAL_EXPR,
//     // AST_DECL := IDENT ":" IDENT
//     //           | IDENT ":" TYPE
//     //           | BLOCK
//     AST_DECL,
//     // AST_DEF  := AST_DECL "=" VAL_EXPR ";"
//     AST_DEF,
//     // AST_ASING := IDENT "=" VAL_EXPR ";"
//     AST_ASING,
//     // AST_STMT := CALL
//     //           | RET
//     //           | BREAK
//     //           |
//     AST_STMT,
//     // AST_CALL := IDENT "(" [VAL_EXPR ["," VAL_EXPR]*] ")"
//     AST_CALL,
//     // AST_RETURN := "ret" [VAL_EXPR]? ";"
//     AST_RETURN,
//
//     // AST_IF   := "if" "(" AST_VAL_EXPR ")" BLOCK
//     AST_IF,
//     // AST_ELSE := "else" BLOCK
//     //           | AST_IF
//     AST_ELSE,
//     // AST_WHILE := "while" "(" AST_VAL_EXPR ")" BLOCK
//     // AST_WHILE,
//     // AST_FOR  := "for" "(" DECL ";" AST_VAL_EXPR ";" AST_VAL_EXPR ")" BLOCK
//     // AST_FOR,
//     // AST_LOOP := "loop" BLOCK
//     AST_LOOP,
//     // AST_BREAK := "break" ";"
//     AST_BREAK,
//     // AST_CONTINUE := "continue" ";"
//     AST_CONTINUE,
//
//     AST__COUNT
// } ast_node_t;

typedef struct AST_Node_s AST_Node;
struct AST_Node_s {
    ast_node_t tag;
    Token tok;
    Location loc;
    size_t child_count;
    AST_Node** children;
};

typedef struct {
    Tokenizer0* tok;
    Null_Pool* identifiers;
    Span_Pool* strings;
    AST_Node* root;

    Token peek;
    int has_peeked;
} AST;

extern int ast_work(AST* const ast);
extern void ast_init(AST* ast, Tokenizer0* tok, Null_Pool* ids, Span_Pool* strs);
extern void ast_print_node(AST_Node* node, int depth);

#endif // !_AST_H


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "tokenizer.h"
#include "utils.h"

// grep -F "AST_Node* ast_" src/ast.c
static AST_Node* ast_new(ast_node_t tag, const Token* tok, const Location* loc);
static void* ast_add_child(AST_Node* parent, AST_Node* child);

static int ast_next_token(AST* ast);
static Token* ast_peek_token(AST* ast);
static int ast_accept_token(AST* ast, token_t expected_type);
static void ast_expect_token(AST* ast, token_t expected_type);

static AST_Node* ast_parse_primary(AST* ast);
static AST_Node* ast_parse_return(AST* ast);
static AST_Node* ast_parse_assing(AST* ast);
static AST_Node* ast_parse_expr(AST* ast);
static AST_Node* ast_parse_type(AST* ast);

void print_error(Token* token, const char* message, const char* source_line)
{
    fprintf(stderr, "carmen:error:%zu:%zu: %s\n", token->loc.row,
        token->loc.col, message);

    if ( source_line ) {
        fprintf(stderr, "%s\n", source_line);

        // Print caret under the column
        for ( size_t i = 1; i < token->loc.col; ++i ) {
            fputc(source_line[i - 1] == '\t' ? '\t' : ' ', stderr);
        }
        fprintf(stderr, "^\n");
    }
}

const char* ast_node_tag_to_str(ast_node_t tag)
{
    // TODO: make just an array...
    switch ( tag ) {
        case AST_ROOT:    return "Root";
        case AST_LIT_INT: return "Lit_int";
        case AST_TYPE:    return "Type";
        case AST_IDENT:   return "Ident";
        case AST_EXPR:    return "Expr";
        case AST_OP_ADD:  return "Op_add";
        case AST_OP_MUL:  return "Op_mul";
        case AST_ASSIGN:  return "Assign";
        case AST_DECL:    return "Decl";
        case AST_RETURN:  return "Return";
        default:          return "Unknown";
    }
}
void ast_print_indent(int depth)
{
    for ( int i = 0; i < depth; ++i ) { printf("|   "); }
}
void ast_print_node(AST_Node* node, int depth)
{
    if ( !node ) {
        ast_print_indent(depth);
        printf("(null)\n");
        return;
    }

    ast_print_indent(depth);

    printf("%s, ", ast_node_tag_to_str(node->tag));

    tok_print(&node->tok);

    printf("\n");

    for ( size_t i = 0; i < node->child_count; ++i ) {
        ast_print_node(node->children[i], depth + 1);
    }
}


void ast_init(AST* ast, Tokenizer0* tok, Null_Pool* ids, Span_Pool* strs)
{
    { // sanity check
        ASSERT(ast != NULL);
        ASSERT(tok != NULL);
        ASSERT(ids != NULL);
        ASSERT(strs != NULL);
    }

    { // setup buff
        *ast = (AST) {
            .root = NULL,
            .tok = tok,
            .identifiers = ids,
            .strings = strs,
        };
    }
}

AST_Node* ast_new(ast_node_t tag, const Token* tok, const Location* loc)
{
    AST_Node* node = calloc(1, sizeof(AST_Node));
    node->tag = tag;
    node->tok = (tok != NULL) ? *tok : (Token) { 0 };
    node->loc = (loc) ? *loc : (tok != NULL) ? tok->loc : (Location) { 0 };
    node->children = NULL;
    node->child_count = 0;
    return node;
}
void* ast_add_child(AST_Node* parent, AST_Node* child)
{
    if ( child == NULL ) { return NULL; }
    void* children = realloc(
        parent->children, sizeof(AST_Node*) * (parent->child_count + 1));
    // TODO die() if fails
    if ( children == NULL ) { return NULL; }
    parent->children = children;
    parent->children[parent->child_count++] = child;
    return children;
}

// next
int ast_next_token(AST* ast)
{
    switch ( tok_next(ast->tok, &ast->peek, ast->identifiers, ast->strings) ) {
        case TOKENIZER_FAIL: return false;
        case TOKENIZER_EOF:
        case TOKENIZER_SUCCESS:
            ast->has_peeked = 1;
            // debug
            printf("---> ");
            tok_print(&ast->peek);
            printf("\n");
            // getchar();
            return true;
        default: UNREACHABLE("???");
    }
}
// get current
Token* ast_peek_token(AST* ast)
{
    if ( !ast->has_peeked ) {
        if ( !ast_next_token(ast) ) { ast->peek.type = TOK_ILLEGAL; }
        ast->has_peeked = 1;
    }
    return &ast->peek;
}
// if current token
int ast_check_token(AST* ast, token_t expected_type)
{
    return ast_peek_token(ast)->type == expected_type;
}
// if current token matches read next
int ast_accept_token(AST* ast, token_t expected_type)
{
    return ast_check_token(ast, expected_type) && ast_next_token(ast);
}
// if current token matches next or and throw error
void ast_expect_token(AST* ast, token_t expected_type)
{
    Token* token = ast_peek_token(ast);
    if ( !ast_check_token(ast, expected_type) ) {
        fprintf(stderr,
            "carmen:error:"
            "%zu:%zu: expected token type '%s', but got '%s'"
            "\n",
            token->loc.row, token->loc.col, tok_get_type_rep(expected_type),
            tok_get_type_rep(token->type));
        tok_print_rep(token);
        exit(1);
    }
    ast_next_token(ast);
}

// TODO: ast_expect_node()...
// TODO: do we clean up generated tree if we fail to fully parse??? I think
// yes...

// NOTE: all the parse X functions check for peek token if they match they might
//       start consuming tokens, so there is no comming back -> we print a cool
//       error and die();

// primary = INT | IDENT
AST_Node* ast_parse_primary(AST* ast)
{
    Token token = *ast_peek_token(ast);
    if ( ast_accept_token(ast, TOK_INTEGER) ) {
        return ast_new(AST_LIT_INT, &token, NULL);
    } else if ( ast_accept_token(ast, TOK_IDENTIFIER) ) {
        return ast_new(AST_IDENT, &token, NULL);
    }
    fprintf(stderr,
        "carmen:error:%zu:%zu: expected primary expr tok, but got '%s'\n",
        token.loc.row, token.loc.col, tok_get_type_rep(token.type));
    tok_print_rep(&token);
    return NULL;
}


// (1) "ret" EXPR ";"
AST_Node* ast_parse_return(AST* ast)
{
    Token base = *ast_peek_token(ast);
    AST_Node* expr = ast_parse_expr(ast);
    ASSERT(expr != NULL);

    AST_Node* node = ast_new(AST_RETURN, &base, NULL);
    ast_add_child(node, expr);
    return node;
}
AST_Node* ast_parse_assing(AST* ast)
{
    Token base = *ast_peek_token(ast);
    AST_Node* expr = ast_parse_expr(ast);
    if ( !expr ) { return NULL; }

    ast_expect_token(ast, TOK_SEMICOLON);

    AST_Node* node = ast_new(AST_RETURN, &base, NULL);
    ast_add_child(node, expr);
    return node;
}

// (1) PRIMARY [OP PRIMARY]*
AST_Node* ast_parse_expr(AST* ast)
{
    Token base = *ast_peek_token(ast);
    AST_Node* v = ast_parse_primary(ast);
    ASSERT(v != NULL);

    // TODO: base.loc is not saving when printing the tree...
    // printf("[%zu:%zu]\n", base.loc.row, base.loc.col);
    AST_Node* node = ast_new(AST_EXPR, NULL, &base.loc);
    // printf("[%zu:%zu]\n", node->loc.row, node->loc.col);
    ast_add_child(node, v);

    // TODO: priority stuff later...
    // TODO: parens...
    while ( 1 ) {
        Token op_tok = *ast_peek_token(ast);

        // for now just '*' and '+' bin ops
        if ( ast_accept_token(ast, TOK_PLUS)
            || ast_accept_token(ast, TOK_STAR) ) {
            int t = op_tok.rep.c == '+' ? AST_OP_ADD : AST_OP_MUL;
            AST_Node* op = ast_new(t, &op_tok, NULL);
            ASSERT(ast_add_child(node, op));
            AST_Node* prim = ast_parse_primary(ast);
            ASSERT(ast_add_child(node, prim));
            continue;
        }
        ast_expect_token(ast, TOK_SEMICOLON);
        break;
    }

    return node;
}

// (1) TYPE ";"
// (2) TYPE = EXPR
AST_Node* ast_parse_type(AST* ast)
{
    Token base = *ast_peek_token(ast);
    if ( !ast_accept_token(ast, TOK_KEYWORD_INT) ) { // INTEGER JUST FOR NOW
        TODO("UNIMPLEMENTED TYPES");
        return NULL;
    }

    AST_Node* node = ast_new(AST_TYPE, &base, NULL);

    if ( ast_check_token(ast, TOK_EQUAL) ) { return node; }

    ast_expect_token(ast, TOK_SEMICOLON);
    return node;
}
AST_Node* parse_assign(AST* ast)
{
    Token base = *ast_peek_token(ast);
    AST_Node* node = ast_new(AST_ASSIGN, &base, NULL);
    ASSERT(node);

    AST_Node* expr = ast_parse_expr(ast);
    ASSERT(expr);
    ast_add_child(node, expr);

    return node;
}

AST_Node* parse_stmt(AST* ast)
{
    Token token = *ast_peek_token(ast);

    if ( ast_accept_token(ast, TOK_KEYWORD_RET) ) {
        return ast_parse_return(ast);
    }

    if ( ast_accept_token(ast, TOK_IDENTIFIER) ) {
        if ( ast_accept_token(ast, TOK_COLON) ) {
            AST_Node* node = ast_new(AST_DECL, &token, NULL);
            ast_add_child(node, ast_parse_type(ast));
            if ( ast_accept_token(ast, TOK_EQUAL) ) {
                ast_add_child(node, ast_parse_expr(ast));
            }
            return node;
        }
        if ( ast_accept_token(ast, TOK_EQUAL) ) {
            AST_Node* node = ast_new(AST_ASSIGN, &token, NULL);
            ast_add_child(node, ast_parse_expr(ast));
            return node;
        }
    }

    LOG_ERRF(
        "%zu:%zu: unexpected start of statement", token.loc.row, token.loc.col);
    tok_print_rep(&token);
    return NULL;
}

int ast_work(AST* ast)
{
    AST_Node* root = ast_new(AST_ROOT, NULL, NULL); // dummy root

    while ( 1 ) {
        printf("\n{NEW NODE} ================================\n");
        if ( ast_peek_token(ast)->type == TOK_EOF ) { break; }

        AST_Node* node = parse_stmt(ast);
        if ( !node ) { return 1; }

        ast_add_child(root, node);
    }
    // ast_print_node(root, 0);
    ast->root = root;
    return 0;
}

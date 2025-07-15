
#include <stdio.h>
#include <stdlib.h>

#include "./ast.h"
#include "./codegen.h"
#include "utils.h"

// grep "^void " ./src/codegen_x86_64.c
extern void gen_binop(FILE* out, AST_Node* node, const char* op_instr);
extern void gen_expr(FILE* out, AST_Node* node);
extern void gen_stmt(FILE* out, AST_Node* node);
extern void code_gen_main(FILE* out, AST_Node* root);


#define INIT_CAP 16
typedef struct Symbol_s {
    void* id;
    int offset;
} Symbol;

typedef struct Symbol_Tab_s {
    size_t cap;
    size_t len;
    Symbol* entries;
} Symbol_Tab;

static void st_init(Symbol_Tab* tab);
static void st_free(Symbol_Tab* tab);
static void st_put(Symbol_Tab* tab, void* id, int offset);
static int st_get(const Symbol_Tab* tab, void* id);
static void st_grow(Symbol_Tab* tab);

void st_init(Symbol_Tab* tab)
{
    tab->cap = INIT_CAP;
    tab->len = 0;
    tab->entries = malloc(sizeof(Symbol) * tab->cap);
}

void st_free(Symbol_Tab* tab)
{
    free(tab->entries);
    tab->entries = NULL;
    tab->len = 0;
    tab->cap = 0;
}

void st_grow(Symbol_Tab* tab)
{
    tab->cap *= 2;
    tab->entries = realloc(tab->entries, sizeof(Symbol) * tab->cap);
}
void st_put(Symbol_Tab* tab, void* id, int offset)
{
    for ( size_t i = 0; i < tab->len; ++i ) {
        if ( tab->entries[i].id == id ) {
            tab->entries[i].offset = offset;
            return;
        }
    }

    if ( tab->len == tab->cap ) { st_grow(tab); }

    tab->entries[tab->len].id = id;
    tab->entries[tab->len].offset = offset;
    tab->len++;
}

int st_get(const Symbol_Tab* tab, void* id)
{
    for ( size_t i = 0; i < tab->len; ++i ) {
        if ( tab->entries[i].id == id ) { return tab->entries[i].offset; }
    }
    LOG_ERRF("undeclared symbol (%p)...", id);
    TODO("PRINT MSG");
    return 0; // TODO: return -1 if not found and print a not declared error if
              // necessary...
}

// TODO: move this globals...
// we could use a stack to place scopes and their offsets...
static int temp_offset = 0;
static Symbol_Tab symtab;

void gen_binop(FILE* out, AST_Node* node, const char* op_instr)
{
    // left
    gen_expr(out, node->children[0]);
    fprintf(out, "    push %%rax\n");

    // right
    gen_expr(out, node->children[1]);
    fprintf(out, "    movl %%eax, %%ebx\n");
    fprintf(out, "    pop %%rax\n");

    fprintf(out, "    %s %%ebx, %%eax\n", op_instr);
}

void gen_expr(FILE* out, AST_Node* node)
{
    if ( node->tag != AST_EXPR ) {
        // TODO: prety print
        fprintf(stderr, "Expected AST_EXPR as top-level in gen_expr\n");
        exit(1);
    }

    if ( node->child_count == 0 ) {
        fprintf(stderr, "Empty expression node\n");
        exit(1);
    }

    AST_Node* lhs = node->children[0];

    switch ( lhs->tag ) {
        case AST_LIT_INT:
            fprintf(out, "    movl $%zu, %%eax\n", lhs->tok.rep.num);
            break;
        case AST_IDENT:
            printf("[%s]\n", lhs->tok.rep.str);
            fprintf(out, "    movl -%d(%%rbp), %%eax\n",
                st_get(&symtab, lhs->tok.rep.id));
            break;
        default:
            fprintf(
                stderr, "Unexpected node in primary position: %d\n", lhs->tag);
            exit(1);
    }

    for ( size_t i = 1; i < node->child_count; i += 2 ) {
        AST_Node* op = node->children[i];
        AST_Node* rhs = node->children[i + 1];

        // Push current %eax (result of lhs so far)
        fprintf(out, "    push %%rax\n");

        // Evaluate rhs into %eax
        switch ( rhs->tag ) {
            case AST_LIT_INT:
                fprintf(out, "    movl $%zu, %%eax\n", rhs->tok.rep.num);
                break;
            case AST_IDENT:
                fprintf(out, "    movl -%d(%%rbp), %%eax\n",
                    st_get(&symtab, rhs->tok.rep.id));
                break;
            default:
                fprintf(stderr, "Unexpected RHS node: %d\n", rhs->tag);
                exit(1);
        }

        // Move RHS into %ebx for the operation
        fprintf(out, "    movl %%eax, %%ebx\n");

        // Pop lhs back into %eax
        fprintf(out, "    pop %%rax\n");

        // Apply the operation
        switch ( op->tag ) {
            case AST_OP_ADD: fprintf(out, "    addl %%ebx, %%eax\n"); break;
            case AST_OP_MUL: fprintf(out, "    imull %%ebx, %%eax\n"); break;
            default:
                fprintf(stderr, "Unknown binary operator: %d\n", op->tag);
                exit(1);
        }
    }
}

void gen_stmt(FILE* out, AST_Node* node)
{
    switch ( node->tag ) {
        case AST_DECL: {
            AST_Node* expr = node->children[1]; // skip type

            temp_offset += 4;
            st_put(&symtab, node->tok.rep.id, temp_offset);
            printf(
                "`-> DECL: id:%p -> off:%d\n", node->tok.rep.id, temp_offset);

            if ( expr != NULL ) {
                gen_expr(out, expr);
                fprintf(out, "    movl %%eax, -%d(%%rbp)\n", temp_offset);
            }

            break;
        }
        case AST_ASSIGN: {
            AST_Node* expr = node->children[0]; // skip type

            gen_expr(out, expr);
            fprintf(out, "    movl %%eax, -%d(%%rbp)\n",
                st_get(&symtab, node->tok.rep.id));
            break;
        }
        case AST_RETURN: {
            gen_expr(out, node->children[0]); // result in %eax
            fprintf(out, "    mov %%rbp, %%rsp\n");
            fprintf(out, "    pop %%rbp\n");
            fprintf(out, "    ret\n");
            break;
        }
        default:
            fprintf(stderr, "Unhandled stmt tag: %d\n", node->tag);
            exit(1);
    }
}

void code_gen_main(FILE* out, AST_Node* root)
{
    st_init(&symtab);

    fprintf(out, "# HEAD: \n");
    fprintf(out, ".global main\n");
    fprintf(out, ".text\n\n");
    fprintf(out, "main:\n");
    fprintf(out, "    push %%rbp\n");
    fprintf(out, "    mov %%rsp, %%rbp\n");
    // TODO: change this stuff...
    fprintf(out, "    sub $64, %%rsp  # Reserve some bytes\n");

    fprintf(out, "\n");
    fprintf(out, "# CODE: \n");
    printf("-----------------------------------------------------------\n");
    for ( size_t i = 0; i < root->child_count; ++i ) {
        fprintf(out, "    # [%zu]\n", i);
        ast_print_node(root->children[i], 0);
        gen_stmt(out, root->children[i]);
        printf("-----------------------------------------------------------\n");
    }
    // fprintf(out, "\n");
    // fprintf(out, "# TAIL: \n");

    // // fallback return
    // fprintf(out, "    movl $0, %%edi\n");
    // fprintf(out, "    call exit\n");
}

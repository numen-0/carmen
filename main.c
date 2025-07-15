
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/ast.h"
#include "src/codegen.h"
#include "src/string_pool.h"
#include "src/tokenizer.h"
#include "src/utils.h"


int main(int argc, char* argv[])
{

    if ( argc != 3 ) {
        fprintf(stderr, "Usage: %s <SRC_FILE> <OUT_FILE>", argv[0]);
        exit(1);
    }

    printf("[CC] --> START\n");
    // char blob[MAIN_CONTEXT_SIZE];
    // Context main_c = context_init(blob, MAIN_CONTEXT_SIZE);

    { // read
        AST ast = { 0 };
        Tokenizer0 tok = { 0 };
        Span_Pool spool = { 0 };
        Null_Pool npool = { 0 };

        { // setup ast
            if ( tok_init(&tok, argv[1]) == TOKENIZER_FAIL ) { exit(1); }

            ASSERT(spool_init(&spool));
            ASSERT(npool_init(&npool));

            ast_init(&ast, &tok, &npool, &spool);
        }

        // Context loop_c = context_lock(main_c);
        // UNUSED(loop_c);

        { // ast + codegen
            printf("[AST] --> START \n");
            if ( ast_work(&ast) ) { exit(EXIT_FAILURE); }
            printf("[AST] <-- END \n");
            // npool_print(ast->identifiers);
            FILE* out = fopen(argv[2], "w");
            ASSERT(out);
            printf("[GEN] --> START \n");
            code_gen_main(out, ast.root);
            printf("[GEN] <-- END \n");
            fclose(out);
        }
        // context_unlock(main_c); // free loop_c
    }
    printf("[CC] <-- END\n");

    return EXIT_SUCCESS;
}

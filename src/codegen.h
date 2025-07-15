#ifndef _CODEGEN_H
#define  _CODEGEN_H

#include <stdio.h>

#include "ast.h"

extern void code_gen_main(FILE* out, AST_Node* root);

#endif // !_CODEGEN_H

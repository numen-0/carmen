#ifndef _SCANNER_H
#define _SCANNER_H

#include "utils.h"
#include <stddef.h>

enum {
    SCANNER_SUCCESS = 0,
    SCANNER_FAIL = 1,
};

typedef struct {
    SIGN_CONTRACT_LL(node);
    size_t len;
    size_t size;
    char* data;
} Line;
VALIDATE_CONTRACT_LL(Line, node)

typedef struct Scanner_s {
    const char* file_name;
    List lines;
    List_Node* current;
} Scanner;

extern int scanner_load(Scanner* const scanner, const char* const file_name);
extern int scanner_free(Scanner* const scanner);

extern Line* scanner_next(Scanner* const scanner);
extern Line* scanner_peek(Scanner* const scanner);
extern int scanner_hasNext(Scanner* const scanner);

#endif // !_SCANNER_H

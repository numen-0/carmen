#ifndef _STRING_POOL_H
#define _STRING_POOL_H

#include <stddef.h>

#include "utils.h"


typedef struct Span_Pool_s {
    List pools;
}Span_Pool;
typedef struct Null_Pool_s {
    List pools;
} Null_Pool;

typedef struct Span_String_s {
    size_t size;
    const char str[];
} Span_String;

extern int spool_init(Span_Pool* spool);
extern Span_String* spool_add(Span_Pool* spool, const char* str, size_t n);
extern void spool_print(const Span_Pool* spool);

extern int npool_init(Null_Pool* npool);
extern const char* npool_add(Null_Pool* npool, const char* str, size_t n);
extern void npool_print(const Null_Pool* npool);

#endif // !_STRING_POOL_H

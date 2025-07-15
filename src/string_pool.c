
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_pool.h"
#include "utils.h"

/*****************************************************************************/

#define POOL_BLOCK_SIZE 1024

typedef struct Pool_s {
    SIGN_CONTRACT_LL(node);
    size_t capacity;
    size_t pivot;
    char data[];
} Pool;
VALIDATE_CONTRACT_LL(Pool, node)

/*****************************************************************************/

static void* pool_get(const Pool* pool, size_t offset)
{
    { // sanity check
        ASSERT(pool != NULL);
    }

    return (void*)&pool->data[offset];
}

/*****************************************************************************/

static Pool* spool_new(Span_Pool* spool, size_t size)
{
    { // sanity check
        ASSERT(spool != NULL);
    }

    Pool* pool = list_node_new(sizeof(Pool) + size);
    if ( spool == NULL ) { return NULL; }

    { // setup pool
        pool->node.next = NULL;
        pool->capacity = size;
        pool->pivot = 0;
    }

    list_push(&spool->pools, (void*)pool);

    return pool;
}
int spool_init(Span_Pool* spool)
{
    list_init(&spool->pools);
    return spool_new(spool, POOL_BLOCK_SIZE) != NULL;
}
static Span_String* spool__add(Pool* pool, const char* str, size_t n)
{

    Span_String* sstr = pool_get(pool, pool->pivot);
    ASSERT(sstr);
    memcpy((char*)&sstr->str, str, n);
    sstr->size = n;
    // TODO: this could overflow size_t...
    pool->pivot += DATA_ROUND_UP(n + sizeof(Span_String), DATA_ALIGN);

    return sstr;
}
Span_String* spool_add(Span_Pool* spool, const char* str, size_t n)
{
    { // sanity check
        ASSERT(spool != NULL);
        ASSERT(str != NULL);
    }

    const size_t N = n + sizeof(Span_String);
    list_foreach(Pool * pool, spool->pools)
    {
        if ( pool->pivot + N <= pool->capacity ) { spool__add(pool, str, n); }
    }

    Pool* pool = spool_new(spool, (POOL_BLOCK_SIZE < N) ? N : POOL_BLOCK_SIZE);
    if ( pool == NULL ) { return NULL; }

    return spool__add(pool, str, n);
}
void spool_print(const Span_Pool* spool)
{
    { // sanity check
        ASSERT(spool != NULL);
    }

    size_t j = 0;
    list_foreach(Pool * pool, spool->pools)
    {
        for ( size_t i = 0; i < pool->pivot; i++ ) {
            Span_String* span = pool_get(pool, i);
            printf("%02zu:%03zu: \"%.*s\"\n", j, i, (int)span->size, span->str);
            i += span->size;
        }
    }
}

/*****************************************************************************/

static Pool* npool_new(Null_Pool* npool, size_t size)
{
    { // sanity check
        ASSERT(npool != NULL);
    }

    Pool* pool = list_node_new(sizeof(Pool) + size);
    if ( npool == NULL ) { return NULL; }

    { // setup pool
        pool->node.next = NULL;
        pool->capacity = size;
        pool->pivot = 0;
    }

    list_push(&npool->pools, (void*)pool);

    return pool;
}
int npool_init(Null_Pool* npool)
{
    list_init(&npool->pools);
    return npool_new(npool, POOL_BLOCK_SIZE) != NULL;
}
const char* npool__add(Pool* pool, const char* str, size_t n)
{

    char* ptr = pool_get(pool, pool->pivot);
    ASSERT(ptr);
    memcpy(ptr, str, n);
    ptr[n] = '\0';
    pool->pivot = pool->pivot + n + 1; // NOTE: no need to align

    return ptr;
}
const char* npool_add(Null_Pool* npool, const char* str, const size_t n)
{
    { // sanity check
        ASSERT(npool != NULL);
        ASSERT(str != NULL);
    }

    // find match
    list_foreach(Pool * pool, npool->pools)
    {
        for ( size_t i = 0; i < pool->pivot; i++ ) {
            const char* pstr = pool_get(pool, i);
            size_t len = strlen(pstr);
            if ( len == n && strncmp(pstr, str, n) == 0 ) { return pstr; }
            i += len;
        }
    }

    // add to pool
    const size_t N = n + 1;
    list_foreach(Pool * pool, npool->pools)
    {
        if ( pool->pivot + N <= pool->capacity ) {
            return npool__add(pool, str, n);
        }
    }

    Pool* pool = npool_new(npool, (POOL_BLOCK_SIZE < N) ? N : POOL_BLOCK_SIZE);
    if ( pool == NULL ) { return NULL; }

    return npool__add(pool, str, n);
}
void npool_print(const Null_Pool* npool)
{
    { // sanity check
        ASSERT(npool != NULL);
    }

    size_t j = 0;
    list_foreach(Pool * pool, npool->pools)
    {
        for ( size_t i = 0; i < pool->pivot; i++ ) {
            const char* str = pool_get(pool, i);
            printf("%02zu:%03zu: \"%s\"\n", j, i, str);
            i += strlen(str);
        }
    }
}

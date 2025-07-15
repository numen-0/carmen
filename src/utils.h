#ifndef _UTILS_H
#define _UTILS_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

// TODO: have a more standard way to print compilation errors vs internal ones
#define _LOG_WARN_P  "carmen:warn:"
#define _LOG_ERROR_P "carmen:error:"

#define LOG_WARN(msg)       fprintf(stderr, _LOG_WARN_P msg "\n")
#define LOG_WARNF(fmt, ...) fprintf(stderr, _LOG_WARN_P fmt "\n", __VA_ARGS__)
#define LOG_ERR(msg)        fprintf(stderr, _LOG_ERROR_P msg "\n")
#define LOG_ERRF(fmt, ...)  fprintf(stderr, _LOG_ERROR_P fmt "\n", __VA_ARGS__)
#define LOG_MSG(msg)        fprintf(stdout, msg "\n")
#define LOG_MSGF(fmt, ...)  fprintf(stdout, fmt "\n", __VA_ARGS__)

#define ASSERT           assert
#define STATIC_ASSERT    _Static_assert
#define UNUSED(var)      ((void)var)
#define TODO(msg)        ASSERT(0 && ("TODO: " msg))
#define UNREACHABLE(msg) ASSERT(0 && ("UNREACHABLE: " msg))

#define DATA_ALIGN              8
#define DATA_ROUND_UP(n, align) (((n) + (align) - 1) & -(align))

/*****************************************************************************/
/* [L]ocation ****************************************************************/
/*****************************************************************************/

// TODO: add filename here and all the uses of Location
typedef struct Location_s {
    const char* file_name;
    size_t col, row;
} Location;

/*****************************************************************************/
/* [B]buffer *****************************************************************/
/*****************************************************************************/

typedef struct Buffer_s {
    size_t count;
    size_t size;
    char data[];
} Buffer;

extern Buffer* buff_init(char* const buff, const size_t size);
extern void buff_lshift(Buffer* const buff, const size_t offset);

/*****************************************************************************/
/* [L]inked [L]ist ***********************************************************/
/*****************************************************************************/

#define SIGN_CONTRACT_LL(name) List_Node name
#define VALIDATE_CONTRACT_LL(type, member) \
    STATIC_ASSERT(offsetof(type, member) == 0, "List_Node must be at offset 0");

/* NOTE: In order to work the struct needs to be on top of the struct.
 *     struct {
 *         List_Node node; // no pointer!!!
 *         ...
 *     };
 */
typedef struct List_Node_s List_Node;
struct List_Node_s {
    List_Node* next;
};
typedef struct List {
    List_Node* head;
    List_Node* tail;
} List;

#define list_foreach(type_item, list)                                          \
    for ( List_Node* _node = (list).head; _node != NULL; _node = _node->next ) \
        for ( type_item = (void*)_node, *_skip = NULL; _skip == NULL; _skip++ )

extern void list_init(List* list);
extern void list_free(List* list);
extern void list_push(List* list, List_Node* node);
extern void* list_peek(const List* list, size_t offset);
extern void* list_node_new(size_t size);

#endif // !_UTILS_H

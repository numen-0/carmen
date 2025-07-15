
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/*****************************************************************************/
/** buffer *******************************************************************/

Buffer* buff_init(char* const buff, const size_t size)
{
    { // sanity check
        ASSERT(buff != NULL);
        ASSERT(size >= sizeof(Buffer));
    }

    Buffer* const buffer = (void*)buff;

    { // setup buff
        buffer->size = size - sizeof(Buffer);
        buffer->count = 0;
    }

    return buffer;
}
void buff_lshift(Buffer* const buff, const size_t offset)
{
    { // sanity check
        ASSERT(buff != NULL);
        ASSERT(offset <= buff->count);
    }

    if ( offset == 0 ) { return; }

    { // shift
        const size_t move = buff->count - offset;
        memmove(&buff->data[0], &buff->data[offset], move);
        buff->count = move;
    }
}

/*****************************************************************************/
/** list *********************************************************************/

void list_init(List* list)
{
    { // sanity check
        ASSERT(list != NULL);
    }

    { // setup buff
        *list = (List) {
            .head = NULL,
            .tail = NULL,
        };
    }
}
void list_free(List* list)
{
    list_foreach(char* node, *list) { free(node); }
    list->tail = NULL;
    list->head = NULL;
}
void list_push(List* list, List_Node* node)
{
    { // sanity check
        ASSERT(list != NULL);
        ASSERT(node != NULL);
    }

    if ( list->head == NULL ) { // empty
        list->head = node;
        list->tail = node;
        return;
    }
    { // push to tail
        list->tail->next = node;
        list->tail = node;
    }
}
void* list_peek(const List* list, size_t offset)
{
    { // sanity check
        ASSERT(list != NULL);
    }

    { // jump n times
        size_t count = 0;
        list_foreach(const List_Node* node, *list)
        {
            if ( offset <= count++ ) { return (void*)node; }
        }
    }
    return NULL;
}
void* list_node_new(size_t size)
{
    { // sanity check
        ASSERT(size >= sizeof(List_Node*));
    }

    List_Node* node = malloc(size);
    node->next = NULL;
    return (void*)node;
}

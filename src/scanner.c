
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "utils.h"


#define LINE_BUF_BATCH_SIZE 80
int get_line(Line* line, FILE* stream)
{
    { // sanity check
        ASSERT(line != NULL);
        ASSERT(stream != NULL);
    }

    char buff[LINE_BUF_BATCH_SIZE + 1];
    size_t n = 0;
    int c = 0;
    line->size = 0;
    line->len = 0;
    line->data = NULL;

    while ( (c = fgetc(stream)) != EOF ) {
        buff[n++] = c;
        if ( c == '\n' ) { break; }

        if ( n == LINE_BUF_BATCH_SIZE ) { // load batch
            const size_t size = line->size + LINE_BUF_BATCH_SIZE;
            char* ptr = realloc(line->data, size);
            if ( ptr == NULL ) {
                free(line->data);
                return -1;
            }
            memcpy(&ptr[line->size], buff, LINE_BUF_BATCH_SIZE);
            line->data = ptr;
            line->size += LINE_BUF_BATCH_SIZE;
            n = 0;
        }
    }

    if ( line->data == NULL && c == EOF ) { return -1; }

    { // load last batch
        char* ptr = realloc(line->data, line->size + n + 1);
        if ( !ptr ) {
            free(line->data);
            return -1;
        }

        memcpy(&ptr[line->size], buff, n);
        line->data = ptr;
        line->size += n;
        line->len = line->size;
        line->data[line->len] = '\0';
    }

    return 0;
}

Line* scanner_next(Scanner* const scanner)
{
    ASSERT(scanner->current != NULL);
    return (Line*)(scanner->current = scanner->current->next);
}
Line* scanner_peek(Scanner* const scanner) { return (Line*)(scanner->current); }
int scanner_hasNext(Scanner* const scanner)
{
    return scanner->current->next != NULL;
}

int scanner_load(Scanner* const scanner, const char* const file_name)
{
    printf("[SCANNER](%s) --> START\n", file_name);
    FILE* f = fopen(file_name, "rb");
    if ( f == NULL ) {
        perror("fopen");
        return SCANNER_FAIL;
    }

    list_init(&scanner->lines);
    scanner->file_name = file_name;

    errno = 0;

    while ( 1 ) {
        Line* line = list_node_new(sizeof(Line));
        if ( line == NULL ) {
            perror("malloc");
            fclose(f);
            scanner_free(scanner);
            return SCANNER_FAIL;
        }

        line->data = NULL;
        line->size = 0;

        if ( get_line(line, f) == -1 ) {
            free(line);
            break;
        }

        list_push(&scanner->lines, (List_Node*)line);
    }

    if ( errno == ENOMEM ) {
        perror("getline");
        scanner_free(scanner);
        fclose(f);
        return SCANNER_FAIL;
    }

    scanner->current = scanner->lines.head;

    printf("[SCANNER](%s) <-- END\n", file_name);

    fclose(f);

    printf("[FILE](%s) --> START\n", file_name);
    int i = 0;
    list_foreach(Line * line, scanner->lines)
    {
        printf("%6d: %s", i++, line->data);
    }
    printf("[FILE](%s) <-- END\n", file_name);

    return SCANNER_SUCCESS;
}


int scanner_free(Scanner* const scanner)
{
    list_foreach(Line * line, scanner->lines)
    {
        free(line->data);
        free(line);
    }
    return SCANNER_SUCCESS;
}

#ifndef _CONFIG_H
#define _CONFIG_H

// TODO: add limmits and check to those:
//          - max nesting
//          - max funtion params
//          - max identifier size
//          - string size?

#define MAIN_CONTEXT_SIZE (1 << 16)

#define FILE_MAX_LINES    (1 << 10)
#define SCANNER_BUFF_SIZE (1 << 10)

#define TOKEN_BUFF_SIZE (1 << 12)
#define TOKEN_MAX_SIZE  (64)

#define STRING_POOL_SIZE     (1 << 12)
#define IDENTIFIER_POOL_SIZE (1 << 10)

#define AST_BUFF_SIZE (1 << 12)

#endif // !_CONFIG_H

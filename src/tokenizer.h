#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

#include "config.h"
#include "scanner.h"
#include "string_pool.h"
#include "utils.h"


typedef enum {
    // invalid
    TOK_ILLEGAL,
    // EOF
    TOK_EOF,
    // composed
    TOK_IDENTIFIER,
    TOK_INTEGER,
    TOK_STRING,
    // char symbols
    TOK_AMPERSAND = '&',
    TOK_AT = '@',
    TOK_BACKSLASH = '\\',
    TOK_CARET = '^',
    TOK_COLON = ':',
    TOK_COMMA = ',',
    TOK_DOLLAR = '$',
    TOK_DOUBLEQUOTE = '"',
    TOK_EQUAL = '=',
    TOK_EXCLAMATION = '!',
    TOK_GRAVE = '`',
    TOK_GREATER = '>',
    TOK_HASH = '#',
    TOK_LBRACE = '{',
    TOK_LBRACKET = '[',
    TOK_LESS = '<',
    TOK_LPAREN = '(',
    TOK_MINUS = '-',
    TOK_PERCENT = '%',
    TOK_PERIOD = '.',
    TOK_PIPE = '|',
    TOK_PLUS = '+',
    TOK_QMARK = '?',
    TOK_QUOTE = '\'',
    TOK_RBRACE = '}',
    TOK_RBRACKET = ']',
    TOK_RPAREN = ')',
    TOK_SEMICOLON = ';',
    TOK_SLASH = '/',
    TOK_STAR = '*',
    TOK_TILDE = '~',
    TOK_UNDERSCORE = '_',
    // compound symbols
    TOK__COMPOUND_START = 128,
    TOK_COMPOUND_ARROW = TOK__COMPOUND_START, // "->"
    TOK_COMPOUND_LSHIFT,                      // "<<"
    TOK_COMPOUND_RSHIFT,                      // ">>"
    TOK_COMPOUND_EQ,                          // "=="
    TOK_COMPOUND_LE,                          // "<="
    TOK_COMPOUND_GE,                          // ">="
    TOK_COMPOUND_NE,                          // "!="
    TOK_COMPOUND_INC,                         // "++"
    TOK_COMPOUND_DEC,                         // "--"
    TOK__COMPOUND_END,
    // keyword
    TOK__KEYWORD_START = 160,
    // - control flow
    TOK_KEYWORD_IF = TOK__KEYWORD_START, // "if"
    TOK_KEYWORD_ELSE,                    // "else"
    TOK_KEYWORD_RET,                     // "ret"
    TOK_KEYWORD_WHILE,                   // "while"
    TOK_KEYWORD_FOR,                     // "for"
    TOK_KEYWORD_BREAK,                   // "break"
    TOK_KEYWORD_CONTINUE,                // "continue"
    // - type specifiers
    TOK_KEYWORD_VOID,  // "void"
    TOK_KEYWORD_INT,   // "int"
    TOK_KEYWORD_CHAR,  // "char"
    TOK_KEYWORD_BLOB,  // "blob"
    TOK_KEYWORD_FLOAT, // "float"
    TOK_KEYWORD_CONST, // "const"
    // - type declarations
    TOK_KEYWORD_FUNC,   // "func"
    TOK_KEYWORD_PROC,   // "proc"
    TOK_KEYWORD_STRUCT, // "struct"
    TOK_KEYWORD_ENUM,   // "enum"
    TOK__KEYWORD_END,
} token_t;

enum {
    TOKENIZER_SUCCESS = 0,
    TOKENIZER_FAIL = 1,
    TOKENIZER_EOF = -1,
};

typedef struct Token_s {
    token_t type;
    union {
        size_t num;
        void* id; // used for symbol maps
        const char* str;
        Span_String* span;
        char c;
    } rep;
    Location loc;
} Token;

typedef struct Tokenizer0_s {
    Location loc;
    Scanner scanner;
} Tokenizer0;

int tok_next(
    Tokenizer0* tok, Token* token, Null_Pool* identifiers, Span_Pool* strings);
int tok_init(Tokenizer0* tok, const char* file_name);

// debug
extern void tok_print(const Token* token);
extern void tok_print_rep(const Token* token);
char* tok_get_type_rep(token_t type);

#endif // !_TOKENIZER_H


#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "string_pool.h"
#include "tokenizer.h"
#include "utils.h"

/*****************************************************************************/

const char* reps[] = {
    // invalid token
    [TOK_ILLEGAL] = "?",
    [TOK_EOF] = "?",
    // composed tokens
    [TOK_IDENTIFIER] = "?",
    [TOK_INTEGER] = "?",
    [TOK_STRING] = "?",
    // compound symbols
    [TOK_COMPOUND_ARROW] = "->",
    [TOK_COMPOUND_LSHIFT] = "<<",
    [TOK_COMPOUND_RSHIFT] = ">>",
    [TOK_COMPOUND_EQ] = "==",
    [TOK_COMPOUND_LE] = "<=",
    [TOK_COMPOUND_GE] = ">=",
    [TOK_COMPOUND_NE] = "!=",
    [TOK_COMPOUND_INC] = "++",
    [TOK_COMPOUND_DEC] = "--",
    // char symbols
    [TOK_AMPERSAND] = "&",
    [TOK_AT] = "@",
    [TOK_BACKSLASH] = "\\",
    [TOK_CARET] = "^",
    [TOK_COLON] = ":",
    [TOK_COMMA] = ",",
    [TOK_DOLLAR] = "$",
    [TOK_DOUBLEQUOTE] = "\"",
    [TOK_EQUAL] = "=",
    [TOK_EXCLAMATION] = "!",
    [TOK_GRAVE] = "`",
    [TOK_GREATER] = ">",
    [TOK_HASH] = "#",
    [TOK_LBRACE] = "{",
    [TOK_LBRACKET] = "[",
    [TOK_LESS] = "<",
    [TOK_LPAREN] = "(",
    [TOK_MINUS] = "-",
    [TOK_PERCENT] = "%",
    [TOK_PERIOD] = ".",
    [TOK_PIPE] = "|",
    [TOK_PLUS] = "+",
    [TOK_QMARK] = "?",
    [TOK_QUOTE] = "'",
    [TOK_RBRACE] = "}",
    [TOK_RBRACKET] = "]",
    [TOK_RPAREN] = ")",
    [TOK_SEMICOLON] = ";",
    [TOK_SLASH] = "/",
    [TOK_STAR] = "*",
    [TOK_TILDE] = "~",
    [TOK_UNDERSCORE] = "_",
    // keywords
    // - control flow
    [TOK_KEYWORD_IF] = "if",
    [TOK_KEYWORD_ELSE] = "else",
    [TOK_KEYWORD_RET] = "ret",
    [TOK_KEYWORD_WHILE] = "while",
    [TOK_KEYWORD_FOR] = "for",
    [TOK_KEYWORD_BREAK] = "break",
    [TOK_KEYWORD_CONTINUE] = "continue",
    // - type specifiers
    [TOK_KEYWORD_VOID] = "void",
    [TOK_KEYWORD_INT] = "int",
    [TOK_KEYWORD_CHAR] = "char",
    [TOK_KEYWORD_BLOB] = "blob",
    [TOK_KEYWORD_FLOAT] = "float",
    [TOK_KEYWORD_CONST] = "const",
    // - type declarations
    [TOK_KEYWORD_FUNC] = "func",
    [TOK_KEYWORD_PROC] = "proc",
    [TOK_KEYWORD_STRUCT] = "struct",
    [TOK_KEYWORD_ENUM] = "enum",
};

/*****************************************************************************/

int match_str(const char* str, const char* buf)
{
    return strncmp(str, buf, strlen(str)) == 0;
}
int skip_space(Tokenizer0* tok)
{
    Line* line = scanner_peek(&tok->scanner);
    Location* loc = &tok->loc;
    do {
        if ( !isspace(line->data[loc->col]) ) { return 0; }
    } while ( ++loc->col < line->len );
    loc->row++;
    loc->col = 0;
    scanner_next(&tok->scanner);
    return 1;
}
void skip_line(Tokenizer0* tok)
{
    tok->loc.row++;
    tok->loc.col = 0;
    scanner_next(&tok->scanner);
}

int catch_symbol(Tokenizer0* tok, Token* token)
{
    Line* line = scanner_peek(&tok->scanner);

    const char* str = &line->data[tok->loc.col];
    if ( !ispunct((unsigned char)*str) ) { return false; }

    const size_t len = line->len - tok->loc.col;
    for ( int i = TOK__COMPOUND_START; i < TOK__COMPOUND_END; i++ ) {
        if ( strncmp(str, reps[i], len) == 0 ) {
            token->type = i;
            token->rep.str = reps[i];
            token->loc = tok->loc;
            tok->loc.col += strlen(reps[i]);
            return true;
        }
    }

    token->type = (token_t)*str;
    token->rep.c = *str;
    token->loc = tok->loc;
    tok->loc.col++;
    return true;
}
int catch_number(Tokenizer0* tok, Token* token)
{
    Line* line = scanner_peek(&tok->scanner);

    const char* str = &line->data[tok->loc.col];
    unsigned char c = *str;
    if ( !isdigit(c) ) { return false; }

    size_t i = 0, val = 0;
    do {
        val = val * 10 + (str[i] - '0'); // TODO: warn overflow
        i++;
    } while ( isdigit(str[i]) );

    token->type = TOK_INTEGER;
    token->loc = tok->loc;
    tok->loc.col += i;
    token->rep.num = val;

    return true;
}
int catch_identifier(Tokenizer0* tok, Token* token, Null_Pool* identifiers)
{
    Line* line = scanner_peek(&tok->scanner);

    const char* str = &line->data[tok->loc.col];
    unsigned char c = *str;
    if ( !isalpha(c) && c != '_' ) { return false; }

    size_t i = 1;
    while ( isalnum(str[i]) || str[i] == '_' ) { i++; }

    token->type = TOK_IDENTIFIER;
    token->loc = tok->loc;
    token->rep.str = npool_add(identifiers, str, i);

    tok->loc.col += i;

    return true;
}
int catch_keyword(Tokenizer0* tok, Token* token)
{
    Line* line = scanner_peek(&tok->scanner);

    const char* str = &line->data[tok->loc.col];
    if ( !isalpha((unsigned char)*str) ) { return false; }

    for ( int i = TOK__KEYWORD_START; i < TOK__KEYWORD_END; i++ ) {
        const size_t len = strlen(reps[i]);

        if ( strncmp(str, reps[i], len) == 0 ) {
            token->type = i;
            token->rep.str = reps[i];
            token->loc = tok->loc;
            tok->loc.col += len;
            return true;
        }
    }
    return false;
}
int catch_string(Tokenizer0* tok, Token* token, Span_Pool* strings)
{
    Line* line = scanner_peek(&tok->scanner);

    const char* str = &line->data[tok->loc.col];
    if ( str[0] != '"' ) { return false; }

    size_t i = 2;
    while ( str[i] != '"' ) {
        if ( str[i] == '\\' ) {
            i += 2;
        } else {
            i++;
        }
        if ( line->len <= i ) {
            LOG_ERRF("%s:%zu:%zu: string not closed", tok->loc.file_name,
                tok->loc.row, tok->loc.col);
            token->type = TOK_ILLEGAL;
            token->loc = tok->loc;
            skip_line(tok);
            return true;
        }
    }

    token->type = TOK_STRING;
    token->loc = tok->loc;
    // TODO: scape
    // TODO check for offsets (-2?)
    token->rep.span = spool_add(strings, str + 1, i - 1);

    tok->loc.col += i;

    return true;
}

int tok_next(
    Tokenizer0* tok, Token* token, Null_Pool* identifiers, Span_Pool* strings)
{
    { // sanity check
        ASSERT(tok != NULL);
        ASSERT(token != NULL);
        ASSERT(identifiers != NULL);
        ASSERT(strings != NULL);
    }

    Line* current = scanner_peek(&tok->scanner);
    while ( (current = scanner_peek(&tok->scanner)) != NULL ) {
        if ( skip_space(tok) ) { continue; }

        printf(
            "TOKENIZER %zu:%zu\n%s", tok->loc.row, tok->loc.col, current->data);

        if ( match_str("//", &current->data[tok->loc.col]) ) {
            skip_line(tok);
            continue;
        }

        // NOTE: '_' counts as a symbol or a identifier... depends on the order
        //       we try to catch it.
        // NOTE: first check keywords or they will count ass identifiers
        if ( !(catch_keyword(tok, token) || catch_symbol(tok, token)
                 || catch_identifier(tok, token, identifiers)
                 || catch_number(tok, token)
                 || catch_string(tok, token, strings)) ) {
            token->loc = tok->loc;
            token->type = TOK_ILLEGAL;
            token->rep.str = reps[TOK_ILLEGAL];
        }

        return TOKENIZER_SUCCESS;
    }

    token->loc = tok->loc;
    token->type = TOK_EOF;
    token->rep.str = reps[TOK_EOF];
    return TOKENIZER_EOF;
}

int tok_init(Tokenizer0* tok, const char* file_name)
{
    { // sanity check
        ASSERT(tok != NULL);
        ASSERT(file_name != NULL);
    }

    tok->loc = (Location) {
        .col = 0,
        .row = 0,
    };
    if ( scanner_load(&tok->scanner, file_name) == SCANNER_FAIL ) {
        return TOKENIZER_FAIL;
    }

    return TOKENIZER_SUCCESS;
}

/*****************************************************************************/

void tok_print(const Token* token)
{
    printf("%02zu:%02zu: ", token->loc.row, token->loc.col);
    if ( TOK__COMPOUND_START <= token->type
        && token->type < TOK__COMPOUND_END ) {
        printf("TOK_COMPOUND(%s)", token->rep.str);
        return;
    }
    if ( TOK__KEYWORD_START <= token->type && token->type < TOK__KEYWORD_END ) {
        printf("TOK_KEYWORD(%s)", token->rep.str);
        return;
    }
    switch ( token->type ) {
        case TOK_ILLEGAL: printf("TOK_ILLEGAL(%c)", token->rep.c); break;
        case TOK_EOF:     printf("TOK_EOF()"); break;
        case TOK_STRING:
            printf("TOK_STRING(\"%.*s\")", (int)token->rep.span->size,
                token->rep.span->str);
            break;
        case TOK_IDENTIFIER:
            printf("TOK_IDENTIFIER(%s)", token->rep.str);
            break;
        case TOK_INTEGER: printf("TOK_INTEGER(%zu)", token->rep.num); break;
        default:          printf("TOK_SYMBOL('%c')", token->rep.c);
    }
}
void tok_print_rep(const Token* token)
{
    if ( (TOK__COMPOUND_START <= token->type && token->type < TOK__COMPOUND_END)
        || (TOK__KEYWORD_START <= token->type
            && token->type < TOK__KEYWORD_END) ) {
        printf("%s", token->rep.str);
        return;
    }
    switch ( token->type ) {
        case TOK_ILLEGAL: printf("'%c'", token->rep.c); break;
        case TOK_EOF:     printf("EOF"); break;
        case TOK_STRING:
            printf(
                "\"%.*s\"", (int)token->rep.span->size, token->rep.span->str);
            break;
        case TOK_IDENTIFIER: printf("%s", token->rep.str); break;
        case TOK_INTEGER:    printf("%zu", token->rep.num); break;
        default:             printf("'%c'", token->rep.c);
    }
}
char* tok_get_type_rep(token_t type)
{
    switch ( type ) {
        // invalid
        case TOK_ILLEGAL:          return "illegal";
        // EOF
        case TOK_EOF:              return "EOF";
        // composed
        case TOK_STRING:           return "string";
        case TOK_IDENTIFIER:       return "identifier";
        case TOK_INTEGER:          return "integer";
        // keyword
        // - control flow
        case TOK_KEYWORD_IF:       return "if";
        case TOK_KEYWORD_ELSE:     return "else";
        case TOK_KEYWORD_RET:      return "ret";
        case TOK_KEYWORD_WHILE:    return "while";
        case TOK_KEYWORD_FOR:      return "for";
        case TOK_KEYWORD_BREAK:    return "break";
        case TOK_KEYWORD_CONTINUE: return "continue";
        // - type specifiers
        case TOK_KEYWORD_VOID:     return "void";
        case TOK_KEYWORD_INT:      return "int";
        case TOK_KEYWORD_CHAR:     return "char";
        case TOK_KEYWORD_BLOB:     return "blob";
        case TOK_KEYWORD_FLOAT:    return "float";
        case TOK_KEYWORD_CONST:    return "const";
        // - type declarations
        case TOK_KEYWORD_FUNC:     return "func";
        case TOK_KEYWORD_PROC:     return "proc";
        case TOK_KEYWORD_STRUCT:   return "struct";
        case TOK_KEYWORD_ENUM:     return "enum";
        // compound symbols
        case TOK_COMPOUND_ARROW:   return "arrow";
        case TOK_COMPOUND_LSHIFT:  return "lshift";
        case TOK_COMPOUND_RSHIFT:  return "rshift";
        case TOK_COMPOUND_EQ:      return "double-equal";
        case TOK_COMPOUND_LE:      return "less-or-equal";
        case TOK_COMPOUND_GE:      return "greater-or-equal";
        case TOK_COMPOUND_NE:      return "not-equal";
        case TOK_COMPOUND_INC:     return "increase";
        case TOK_COMPOUND_DEC:     return "decrease";
        // char symbols
        case TOK_AMPERSAND:        return "ampersand";
        case TOK_AT:               return "at";
        case TOK_BACKSLASH:        return "backslash";
        case TOK_CARET:            return "caret";
        case TOK_COLON:            return "colon";
        case TOK_COMMA:            return "comma";
        case TOK_DOLLAR:           return "dollar";
        case TOK_DOUBLEQUOTE:      return "doublequote";
        case TOK_EQUAL:            return "equal";
        case TOK_EXCLAMATION:      return "exclamation";
        case TOK_GRAVE:            return "grave";
        case TOK_GREATER:          return "greater";
        case TOK_HASH:             return "hash";
        case TOK_LBRACE:           return "lbrace";
        case TOK_LBRACKET:         return "lbracket";
        case TOK_LESS:             return "less";
        case TOK_LPAREN:           return "lparen";
        case TOK_MINUS:            return "minus";
        case TOK_PERCENT:          return "percent";
        case TOK_PERIOD:           return "period";
        case TOK_PIPE:             return "pipe";
        case TOK_PLUS:             return "plus";
        case TOK_QMARK:            return "qmark";
        case TOK_QUOTE:            return "quote";
        case TOK_RBRACE:           return "rbrace";
        case TOK_RBRACKET:         return "rbracket";
        case TOK_RPAREN:           return "rparen";
        case TOK_SEMICOLON:        return "semicolon";
        case TOK_SLASH:            return "slash";
        case TOK_STAR:             return "star";
        case TOK_TILDE:            return "tilde";
        case TOK_UNDERSCORE:       return "underscore";
        default:                   UNREACHABLE("???");
    }
}

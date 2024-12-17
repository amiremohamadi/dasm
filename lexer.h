#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define FREE_TKN(tkn) do { \
    if (tkn.type == TKN_IDENT || tkn.type == TKN_INSTR || \
            tkn.type == TKN_REG) { \
        free(tkn.data); \
    } \
} while (0)

typedef enum {
    TKN_EOF,
    TKN_NUM,
    TKN_IDENT,
    TKN_INSTR,
    TKN_REG,
    TKN_SYM,
} tokentype_t;

typedef enum {
    SYM_COLON,
    SYM_COMMA,
    SYM_COMMENT,
    SYM_UNKNOWN,
    SYM_LBRACE,
    SYM_RBRACE,
} symtype_t;

typedef struct {
    tokentype_t type;
    void *data;
} token_t;

typedef struct {
    char *buf;
    size_t i;
} lexer_t;

void debug(token_t tkn);
token_t next_token(lexer_t *lex);

#endif // LEXER_H

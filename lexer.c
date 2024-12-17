#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"

#define IS_INSTR(arg) ( \
    strcmp((arg), "mov") == 0     || \
    strcmp((arg), "add") == 0     || \
    strcmp((arg), "jmp") == 0     || \
    strcmp((arg), "syscall") == 0 || \
    strcmp((arg), "ret") == 0        \
)

#define IS_REG(arg) ( \
    strcmp((arg), "rax") == 0 || \
    strcmp((arg), "rbx") == 0 || \
    strcmp((arg), "rcx") == 0 || \
    strcmp((arg), "rdx") == 0 || \
    strcmp((arg), "rsi") == 0 || \
    strcmp((arg), "rdi") == 0 || \
    strcmp((arg), "eax") == 0 || \
    strcmp((arg), "ebx") == 0 || \
    strcmp((arg), "ecx") == 0 || \
    strcmp((arg), "edx") == 0 || \
    strcmp((arg), "esi") == 0 || \
    strcmp((arg), "edi") == 0 || \
    strcmp((arg), "al") == 0  || \
    strcmp((arg), "bl") == 0  || \
    strcmp((arg), "cl") == 0  || \
    strcmp((arg), "dl") == 0     \
)

int is_space(char c) {
    return c == ' ' || c == '\n';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int is_ident(char c) {
    return (c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'z') || c == '.' || c == '_';
}

void consume_whitespace(lexer_t *lex) {
    while (lex->buf[lex->i] != '\0' && is_space(lex->buf[lex->i])) {
        lex->i++;
    }
}

token_t consume_sym(lexer_t *lex) {
    symtype_t st;

    switch (lex->buf[lex->i++]) {
        case ',':
            st = SYM_COMMA;
            break;
        case ':':
            st = SYM_COLON;
            break;
        case '[':
            st = SYM_LBRACE;
            break;
        case ']':
            st = SYM_RBRACE;
            break;
        case ';':
            while (lex->buf[lex->i] != '\0' && lex->buf[lex->i] != '\n') {
                lex->i++;
            }
            st = SYM_COMMENT;
            break;
        default:
            st = SYM_UNKNOWN;
    }

    return (token_t){
        .type = TKN_SYM,
        .data = &st,
    };
}

token_t consume_ident(lexer_t *lex) {
    char *data;
    size_t n = 0;
    tokentype_t type = TKN_IDENT;

    while (lex->buf[lex->i + n] != '\0' &&
            is_ident(lex->buf[lex->i + n])) {
        n++;
    }

    data = malloc(n + 1);
    data[n] = '\0';
    strncpy(data, lex->buf + lex->i, n);

    lex->i += n;

    if (IS_INSTR(data)) {
        type = TKN_INSTR;
    }
    if (IS_REG(data)) {
        type = TKN_REG;
    }

    return (token_t){
        .data = data,
        .type = type,
    };
}

token_t consume_num(lexer_t *lex) {
    int num = 0;

    while (lex->buf[lex->i] != '\0' && is_digit(lex->buf[lex->i])) {
        num = (num * 10) + (lex->buf[lex->i] - '0');
        lex->i++;
    }

    return (token_t){
        .data = &num,
        .type = TKN_NUM,
    };
}

token_t next_token(lexer_t *lex) {
    token_t tkn;

    consume_whitespace(lex);

    if (lex->buf[lex->i] == '\0') {
        tkn.type = TKN_EOF;
        return tkn;
    }

    if (is_digit(lex->buf[lex->i])) {
        return consume_num(lex);
    }

    if (is_ident(lex->buf[lex->i])) {
        return consume_ident(lex);
    }

    return consume_sym(lex);
}

void debug(token_t tkn) {
    switch (tkn.type) {
        case TKN_EOF:
            printf("TOKEN_EOF\n");
            break;
        case TKN_NUM:
            printf("TOKEN_NUM(%d)\n", *(int *)tkn.data);
            break;
        case TKN_SYM:
            switch (*(int *)tkn.data) {
                case SYM_COLON:
                    printf("TOKEN_SYM(COLON)\n");
                    break;
                case SYM_COMMA:
                    printf("TOKEN_SYM(COMMA)\n");
                    break;
                case SYM_COMMENT:
                    printf("TOKEN_SYM(COMMENT)\n");
                    break;
                case SYM_LBRACE:
                    printf("TOKEN_SYM(LBRACE)\n");
                    break;
                case SYM_RBRACE:
                    printf("TOKEN_SYM(RBRACE)\n");
                    break;
                default:
                    printf("TOKEN_SYM(UNKNOWN)\n");
                    break;
            }
            break;
        case TKN_IDENT:
            printf("TOKEN_IDENT(%s)\n", (char *)tkn.data);
            break;
        case TKN_INSTR:
            printf("TOKEN_INSTR(%s)\n", (char *)tkn.data);
            break;
        case TKN_REG:
            printf("TOKEN_REG(%s)\n", (char *)tkn.data);
            break;
        default:
            printf("TOKEN_UNKNOWN\n");
            break;
    }
}

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#define IS_BINOP(arg) ( \
    arg == INSTR_MOV \
)

#define FREE_INST(ins) do { \
    if (IS_BINOP(ins.type)) { \
        binary_op_t *op = (binary_op_t *)ins.data; \
        free((op_t *)op->op1); \
        free((op_t *)op->op2); \
        free(op); \
    } \
} while (0)

typedef enum {
    INSTR_NOP,
    INSTR_MOV,
    INSTR_RET,
    INSTR_SYSCALL,
    PARSER_EOF,
} instrtype_t;

typedef struct {
    instrtype_t type;
    void *data;
} instr_t;

typedef enum {
    OP_REG,
    OP_LABEL,
    OP_INT,
    OP_UNKNOWN,
} optype_t;

typedef struct {
    optype_t type;
    void *data;
} op_t;

typedef struct {
    op_t *op1;
    op_t *op2;
} binary_op_t;

instr_t next_instr(lexer_t *lex);
size_t encode(instr_t *ins, unsigned char *buf);

#endif // PARSER_H

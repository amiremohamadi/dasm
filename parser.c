#include <string.h>
#include <stdlib.h>

#include "parser.h"

int regord(const char *reg);

op_t *parse_op(lexer_t *lex) {
    token_t tkn = next_token(lex);

    op_t *op = malloc(sizeof(op_t));
    op->data = tkn.data;
    
    switch (tkn.type) {
        case TKN_NUM:
            op->type = OP_INT;
            break;

        case TKN_IDENT:
            op->type = OP_LABEL;
            break;

        case TKN_REG:
            op->type = OP_REG;
            break;

        default:
            op->type = OP_UNKNOWN;
    }

    return op;
}

binary_op_t *parse_bin_op(lexer_t *lex) {
    binary_op_t *bop = malloc(sizeof(binary_op_t));

    bop->op1 = parse_op(lex);
    next_token(lex); // TODO: EXPECT(comma)
    bop->op2 = parse_op(lex);

    return bop;
}

instr_t next_instr(lexer_t *lex) {
    instr_t instr;
    token_t tkn;

    // ignore comments
    do {
        tkn = next_token(lex);
    } while (tkn.type == TKN_SYM && *((symtype_t *)tkn.data) == SYM_COMMENT);

    switch (tkn.type) {
        case TKN_EOF:
            instr.type = PARSER_EOF;
            break;

        case TKN_INSTR: {
            if (strcmp(tkn.data, "ret") == 0) {
                instr.type = INSTR_RET;
            }
            if (strcmp(tkn.data, "syscall") == 0) {
                instr.type = INSTR_SYSCALL;
            }
            if (strcmp(tkn.data, "mov") == 0) {
                instr.type = INSTR_MOV;
            }
            if (strcmp(tkn.data, "ret") == 0) {
                instr.type = INSTR_RET;
            }
            if (strcmp(tkn.data, "jmp") == 0) {
                instr.type = INSTR_JMP;
            }
            if (strcmp(tkn.data, "global") == 0) {
                instr.type = INSTR_GLOBAL_LABEL;
            }
            break;
        }

        case TKN_IDENT: {
            char *value = (char *)tkn.data;
            tkn = next_token(lex);
            if (tkn.type == TKN_SYM && *((symtype_t *)tkn.data) == SYM_COLON) {
                instr.type = INSTR_LABEL;
                instr.data = value;
            }
            break;
        }

        /* default: */
    }

    if (IS_BINOP(instr.type)) {
        instr.data = parse_bin_op(lex);
    }
    if (instr.type == INSTR_JMP ||
            instr.type == INSTR_GLOBAL_LABEL) {
        instr.data = parse_op(lex);
    }

    /* FREE_TKN(tkn); */
    return instr;
}

void encode(asm_t *asmblr, instr_t *ins, unsigned char *buf) {
    buf += asmblr->offset;
    asmblr->offset += instr_off(ins);

    switch (ins->type) {
        case INSTR_MOV: {
            binary_op_t *op = (binary_op_t *)ins->data;
            if (op->op1->type == OP_REG && op->op2->type == OP_INT) {
                buf[0] = 0xb8 + regord((char *)op->op1->data);
                memcpy(buf + 1, (int *)op->op2->data, sizeof(int));
            }
            break;
        }
        case INSTR_JMP: {
            op_t *op = (op_t *)ins->data;
            if (op->type == OP_LABEL) {
                size_t offset = 0;
                buf[0] = 0xeb;

                symentry_t *s = asmblr->syms;
                while (s != NULL) {
                    if (strcmp(s->name, op->data) == 0) {
                        offset = s->offset - asmblr->offset;
                        break;
                    }

                    s = s->next;
                }

                memcpy(buf + 1, &offset, sizeof(int));
            }
            break;
        }
        case INSTR_SYSCALL: {
            buf[0] = 0x0f;
            buf[1] = 0x05;
            break;
        }
        case INSTR_RET: {
            buf[0] = 0xc3;
            break;
        }
    }
}

int instr_off(instr_t *ins) {
    switch (ins->type) {
        case INSTR_RET:
        case INSTR_NOP:
            return 1;
        case INSTR_SYSCALL:
        case INSTR_JMP:
            return 2;
        case INSTR_MOV:
            return 1 + sizeof(int);
        default:
            return 0;
    }
}

int regord(const char *reg) {
    if (strcmp(reg, "rax") == 0 ||
            strcmp(reg, "eax") == 0 ||
            strcmp(reg, "al") == 0) {
        return 0;
    }
    if (strcmp(reg, "rcx") == 0 ||
            strcmp(reg, "ecx") == 0 ||
            strcmp(reg, "cl") == 0) {
        return 1;
    }
    if (strcmp(reg, "rdx") == 0 ||
            strcmp(reg, "edx") == 0 ||
            strcmp(reg, "dl") == 0) {
        return 2;
    }
    if (strcmp(reg, "rbx") == 0 ||
            strcmp(reg, "ebx") == 0 ||
            strcmp(reg, "bl") == 0) {
        return 3;
    }
    if (strcmp(reg, "rsp") == 0 ||
            strcmp(reg, "esp") == 0) {
        return 4;
    }
    if (strcmp(reg, "rbp") == 0 ||
            strcmp(reg, "ebp") == 0) {
        return 5;
    }
    if (strcmp(reg, "rsi") == 0 ||
            strcmp(reg, "esi") == 0) {
        return 6;
    }
    if (strcmp(reg, "rdi") == 0 ||
            strcmp(reg, "edi") == 0) {
        return 7;
    }

    return 0;
}
